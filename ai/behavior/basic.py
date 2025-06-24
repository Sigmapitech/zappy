import logging
import random
from typing import Optional

from .job_template import JobTemplate

logger = logging.getLogger(__name__)

class Basic(JobTemplate):
    """
    Simple autonomous player that collects resources and evolves independently.
    Good fallback behavior when no coordination is available.
    """

    def __init__(self, player):
        super().__init__(player)
        self.job_data = {
            "target_resources": {},
            "movement_pattern": "random", # could have others like line, spiral, etc.
            "last_broadcast": 0,
            "evolution_announced": False,
            "food_priority_threshold": 3, # switch to _focus_on_food() below this
            "resource_priority": [        # order of resource importance
                "linemate", "deraumere", "sibur", 
                "mendiane", "phiras", "thystame"
            ]
        }
        self._update_target_resources()

    def _update_target_resources(self):
        """Update what resources we need based on next evolution"""
        next_level = self.player.level + 1
        if str(next_level) in self.player.elevation_requirements:
            requirements = self.player.elevation_requirements[str(next_level)]
            
            # compute what's left for the incantations
            for resource, needed in requirements.items():
                if resource != "players":
                    current = self.player.resources.get(resource, 0)
                    self.job_data["target_resources"][resource] = max(0, needed - current)

    async def execute_behavior(self):
        """Main behavior loop for basic player"""
        
        await self.player.get_inventory()
        self._update_target_resources()
        
        # switch to food searching if running low
        if self.player.food_stock < self.job_data["food_priority_threshold"]:
            await self._focus_on_food()
        else:
            await self._collect_resources()

        if self.player.can_evolve():
            if not self.job_data["evolution_announced"]:
                await self.player.broadcast(f"evolution_ready {self.player.level}")
                self.job_data["evolution_announced"] = True
            
            await self.player.evolve()
            if self.player.level > self.job_data.get("last_level", 1):
                self.job_data["evolution_announced"] = False
                self.job_data["last_level"] = self.player.level
        
        # heartbeat
        if self.player.counter % 25 == 0:
            await self._broadcast_status()

    async def _focus_on_food(self):
        """Focus on food collection"""
        look_response = await self.player.look()
        tiles = self.player.handle_look_response(look_response)
        
        # Look for food in visible tiles
        food_found = False
        for i, tile in enumerate(tiles):
            if "food" in tile:
                if i == 0: # Food on current tile
                    await self.player.take("food")
                    self.player.food_stock += 1
                    food_found = True
                else:
                    # Move towards food
                    await self._move_to_tile(i)
                    food_found = True
                break
        
        if not food_found:
            await self._random_movement()

    async def _collect_resources(self):
        """Collect resources based on priority and needs"""
        look_response = await self.player.look()
        tiles = self.player.handle_look_response(look_response)
        
        best_tile = None
        best_priority = float('inf')

        for i, tile in enumerate(tiles):
            for j, resource in enumerate(self.job_data["resource_priority"]):
                if (resource in tile and 
                    self.job_data["target_resources"].get(resource, 0) > 0):
                    if j < best_priority:
                        best_priority = j
                        best_tile = i
                        break
        
        if best_tile is not None:
            if best_tile == 0: await self._take_available_resources(tiles[0])
            else: await self._move_to_tile(best_tile)
        else:
            await self._movement_pattern()

    async def _take_available_resources(self, tile):
        """Take all useful resources from current tile"""
        for resource in ["food"] + self.job_data["resource_priority"]:
            if resource in tile:
                await self.player.take(resource)
                if resource == "food":
                    self.player.food_stock += 1
                else:
                    self.player.resources[resource] += 1

    async def _move_to_tile(self, tile_index):
        """Move towards a specific tile"""
        # should work once triangulation's working :)
        
        if tile_index == 1:
            await self.player.move_forward()
        elif tile_index == 2:
            await self.player.turn_left()
            await self.player.move_forward()
        elif tile_index == 3:
            await self.player.turn_right()
            await self.player.move_forward()
        else:
            await self.player.turn_right()
            await self.player.turn_right()
            await self.player.move_forward()

    async def _movement_pattern(self):
        """Execute movement pattern for exploration"""
        pattern = self.job_data["movement_pattern"]
        
        if pattern == "random":
            await self._random_movement()
        # add more?

    async def _random_movement(self):
        """Random movement for exploration"""
        action = random.choice(["forward", "turn_left", "turn_right"])
        
        if action == "forward": await self.player.move_forward()
        elif action == "turn_left": await self.player.turn_left()
        elif action == "turn_right": await self.player.turn_right()
        else: await self.player.turn_right()

    async def _broadcast_status(self):
        """Broadcast current status"""
        needed = sum(self.job_data["target_resources"].values())
        await self.player.broadcast(
            f"basic_status level:{self.player.level} needs:{needed} food:{self.player.food_stock}"
        )

    def should_transition(self) -> Optional[str]:
        """Check if should transition to another job"""
        
        # If an elder appears and assigns us (on_message_received), we might also transition
        
        # If we become high level and no elder exists, consider becoming elder
        if (self.player.level >= 4 and 
            not self.player.job_manager.has_elder() and
            random.random() < 0.1):  # 10% chance to step up
            return "elder"
        
        return None

    async def on_message_received(self, direction: int, message):
        """Handle incoming messages"""
        content = message.content.strip()
        
        if content.startswith("assign_job"):
            parts = content.split(" ")
            if len(parts) >= 3:
                target_id = int(parts[1])
                job_name = parts[2]
                
                if target_id == self.player.id_:
                    # reassigned by the elder
                    self.job_data["assigned_job"] = job_name
        
        elif content.startswith("evolution_call"):
            await self.player.broadcast(f"evolution_response {self.player.id_}")
        elif content.startswith("status_report_request"):
            await self._broadcast_status()
        
        elif content.startswith("resource_request"):
            parts = content.split(" ")
            if len(parts) >= 3:
                resource = parts[1]
                amount = int(parts[2])
                
                # share resources if extra
                # TODO: move towards the broadcast
                if self.player.resources.get(resource, 0) > self.job_data["target_resources"].get(resource, 0):
                    extra = self.player.resources[resource] - self.job_data["target_resources"][resource]
                    can_share = min(extra, amount)
                    
                    if can_share > 0:
                        await self.player.set(resource)
                        await self.player.broadcast(f"resource_available {resource}")

    async def on_job_start(self):
        """Initialize basic job"""
        await super().on_job_start()
        self.job_data["movement_pattern"] = random.choice(["random"])

    def get_job_info(self):
        """Return basic job information"""
        info = super().get_job_info()
        info.update({
            "target_resources": self.job_data["target_resources"],
            "food_stock": self.player.food_stock,
            "evolution_ready": self.player.can_evolve()
        })
        return info
