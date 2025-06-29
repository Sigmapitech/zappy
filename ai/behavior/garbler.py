import logging
import random
import re
from typing import Optional, List

from .job_template import *

logger = logging.getLogger(__name__)

class Garbler(JobTemplate):
    """
    Goes away from his civilization to immerse himself in another one and garble their communications.
    Infiltrator that disrupts enemy team communications.
    """
    
    def __init__(self, player):
        super().__init__(player)
        self.job_data = {
            "home_team_key": None,       # team's encryption key
            "intercepted_messages": [],  # enemy messages we've heard
            "garble_mode": "raven",      # current disruption mode
            "mode_weights": {            # probability weights for each garbling mode
                "raven":   50,  # 50% - Send garbage
                "mimicry": 40,  # 40% - Repeat messages
                "parrot":  10   # 10% - Modified repeats
            },
            "last_mode_switch": 0,
            "messages_sent": 0,
            "survival_priority": False,  # switch to survival if health low
            "home_direction": None,      # direction back to friendly territory
            "noise_characters": [        # noise characters for garbage messages
                "\ufeff", "\\x00", "\\xff", "\\x7f", "\\x1b", "\\x08", 
                "�", "♠", "§", "¤", "¶", "‼", "¿", "¡"
            ]
        }
    
    async def execute_behavior(self):
        """Main garbler behavior"""
        
        # check food supply
        await self.player.get_inventory()
        if self.player.food_stock <= CRITICAL_FOOD_THOLD:
            self.job_data["survival_priority"] = True
        elif self.player.food_stock >= (CRITICAL_FOOD_THOLD * 2):
            self.job_data["survival_priority"] = False
        
        if self.job_data["survival_priority"]:
            await self._survival_behavior()
        else:
            await self._infiltration_behavior()
        
        # switch modes periodically
        if self.player.counter - self.job_data["last_mode_switch"] > 15:
            self._switch_garble_mode()
        await self._send_disruptive_message()
    
    async def _survival_behavior(self):
        """Basic survival when health is low"""
        await self.player.search_food()
    
    async def _infiltration_behavior(self):
        """Main infiltration and disruption behavior"""
        
        # move away from home territory
        if self.job_data["infiltration_depth"] < random.randint(10, 20):
            await self._move_away_from_home()
            self.job_data["infiltration_depth"] += 1
        else:
            # deep enough, start disruption patterns
            await self._disruption_movement()
    
    async def _move_away_from_home(self):
        """Move away from friendly territory"""
        # would probably need better navigation than just going away
        # maybe follow similar messages (based on structure? would need
        # another function)
        
        if self.job_data["home_direction"] is None:
            # pick random direction as "away from home"
            self.job_data["home_direction"] = random.choice(["north", "south", "east", "west"])
        # TODO: change orientation
        await self.player.move_forward()
        
        # random exploration movement
        if random.random() < 0.2:
            if random.choice([True, False]):
                await self.player.turn_left()
            else:
                await self.player.turn_right()
    
    async def _disruption_movement(self):
        """Movement pattern designed to maximize disruption"""
        
        # move in patterns that intersect likely enemy paths
        pattern = random.choice(["circular", "random"])
        
        if pattern == "circular":
            await self._circular_movement()
        else:
            await self.player.random_movement()
    
    async def _circular_movement(self):
        """Move in circular patterns"""
        if not hasattr(self, "_circle_step"):
            self._circle_step = 0
        
        if self._circle_step % 8 < 4:
            await self.player.move_forward()
        else:
            await self.player.turn_right()
        
        self._circle_step += 1
    
    def _switch_garble_mode(self):
        """Switch between different garbling modes"""
        
        # weighted random selection
        total_weight = sum(self.job_data["mode_weights"].values())
        rand = random.randint(1, total_weight)
        
        current_weight = 0
        for mode, weight in self.job_data["mode_weights"].items():
            current_weight += weight
            if rand <= current_weight:
                self.job_data["garble_mode"] = mode
                break
        
        self.job_data["last_mode_switch"] = self.player.counter
        logger.debug(f"Garbler switched to mode: {self.job_data['garble_mode']}")
    
    async def _send_disruptive_message(self):
        """Send a disruptive message based on current mode"""
        
        # DO NOT spam too frequently
        if self.job_data["messages_sent"] % 5 != 0:
            self.job_data["messages_sent"] += 1
            return
        
        mode = self.job_data["garble_mode"]
        
        if mode == "raven": await self._send_raven_message()
        elif mode == "mimicry": await self._send_mimicry_message()
        elif mode == "parrot": await self._send_parrot_message()
        
        self.job_data["messages_sent"] += 1
    
    async def _send_raven_message(self):
        """Send incomprehensible garbage"""
        
        length = random.randint(50, 75)
        garbage = ""
        for _ in range(length):
            if random.random() < 0.4:
                garbage += random.choice(self.job_data["noise_characters"])
            else:
                garbage += chr(random.randint(0, 255))
        await self.player.broadcast(garbage, to_encrypt=False)
    
    async def _send_mimicry_message(self):
        """Repeat a recently intercepted message"""
        
        if not self.job_data["intercepted_messages"]:
            return await self._send_raven_message()

        message = random.choice(self.job_data["intercepted_messages"][-5:])
        await self.player.broadcast(message, to_encrypt=False)
    
    async def _send_parrot_message(self):
        """Send a modified version of intercepted message"""
        
        if not self.job_data["intercepted_messages"]:
            return await self._send_raven_message()
        
        original = random.choice(self.job_data["intercepted_messages"][-5:])
        modified = self._modify_message(original)
        
        await self.player.broadcast(modified, to_encrypt=False)
    
    def _modify_message(self, message: str) -> str:
        """Modify a message to create confusion by altering numbers"""

        message = re.sub(r'\d+', "99999999999999999999999", message)
        return message
    
    def should_transition(self) -> Optional[str]:
        """Check if should transition jobs"""
        
        # return to base if critically low on food
        if self.player.food_stock <= CRITICAL_FOOD_THOLD:
            return "basic"
        
        # if we've been out too long, consider returning
        if self.job_data["infiltration_depth"] > 100:
            return "collector"  # bring back stones and food (can't get enough!)
        
        return None
    
    async def on_message_received(self, direction: int, message):
        """Handle incoming messages - collect intelligence"""
        content = message.content.strip()
        
        # store intercepted messages for mimicry
        if len(content) > 0 and content not in self.job_data["intercepted_messages"]:
            self.job_data["intercepted_messages"].append(content)
            
            # keep only recent messages
            if len(self.job_data["intercepted_messages"]) > 20:
                self.job_data["intercepted_messages"] = self.job_data["intercepted_messages"][-15:]

    async def on_job_start(self):
        """Initialize garbler job"""
        await super().on_job_start()
        self.job_data["home_team_key"] = self.player._SecretivePlayer__secret
        logger.info(f"Player {self.player.id_} started infiltration mission")
    
    async def on_job_end(self):
        """Clean up garbler job"""
        await super().on_job_end()
    
    def get_job_info(self):
        """Return garbler-specific information"""
        info = super().get_job_info()
        info.update({
            "garble_mode": self.job_data["garble_mode"],
            "infiltration_depth": self.job_data["infiltration_depth"],
            "messages_intercepted": len(self.job_data["intercepted_messages"]),
            "enemies_tracked": len(self.job_data["enemy_patterns"])
        })
        return info
