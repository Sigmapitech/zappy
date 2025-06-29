import asyncio
import logging
from typing import Optional

from .job_template import JobTemplate

logger = logging.getLogger(__name__)

class Newcomer(JobTemplate):
    """
    Default job for all newly forked players.
    Listens for instructions from Elder + basic survival.
    """
    
    def __init__(self, player):
        super().__init__(player)
        self.job_data = {
            "elder_id": None,
            "elder_location": None,
            "waiting_for_assignment": True,
            "survival_mode": False,
            "ticks_without_elder": 0,
            "max_ticks_without_elder": 50  # become basic if no elder found
        }
    
    async def execute_behavior(self):
        """Main behavior: wait for elder assignment or survive independently"""
        
        # "i'm a newcomer looking for assignment!"
        if self.job_data["waiting_for_assignment"]:
            await self.player.broadcast(f"newcomer_seeking_assignment {self.player.level}")
            self.job_data["ticks_without_elder"] += 1

        # if no elder response after some time, enter survival mode
        self.job_data["ticks_without_elder"] += 1
        if (self.job_data["ticks_without_elder"] > self.job_data["max_ticks_without_elder"] 
            and not self.job_data["elder_id"]):
            self.job_data["survival_mode"] = True
            logger.debug(f"Newcomer {self.player.id_} entering survival mode")
        
        # basic survival behavior
        if self.job_data["survival_mode"] or not self.job_data["elder_id"]:
            await self._focus_on_food()
    
    async def _focus_on_food(self):
        """Focus on food collection"""
        look_response = await self.player.look()
        tiles = self.player.handle_look_response(look_response)
        
        # look for food in visible tiles
        food_found = False
        for i, tile in enumerate(tiles):
            if "food" in tile:
                if i == 0:
                    await self.player.take("food")
                    self.player.food_stock += 1
                    food_found = True
                else:
                    await self._move_to_tile(i)
                    food_found = True
                break
        if not food_found:
            await self._random_movement()

    def should_transition(self) -> Optional[str]:
        """Check if should transition to another job"""
        
        # get job assigned by Elder
        if "assigned_job" in self.job_data:
            return self.job_data["assigned_job"]
        
        # if nothing is given by the Elder, become basic eventually
        if (self.job_data["survival_mode"] and 
            self.job_data["ticks_without_elder"] > self.jobs_data["max_ticks_without_elder"] * 2):
            return "basic"
        
        # if no elder exists and we're high level, step up
        if (not self.player.job_manager.has_elder() and 
            self.player.level >= 3 and 
            self.job_data["ticks_without_elder"] > 30):
            return "elder"
        
        return None
    
    async def on_message_received(self, direction: int, message):
        """Handle messages from other players"""
        content = message.content.strip()
        
        # Elder identification
        if content.startswith("elder_announce"):
            parts = content.split(" ")
            if len(parts) >= 2:
                elder_id = int(parts[1])
                self.job_data["elder_id"] = elder_id
                self.job_data["ticks_without_elder"] = 0
                logger.debug(f"Newcomer {self.player.id_} found elder: {elder_id}")
        
        # job assignment from Elder
        elif content.startswith("assign_job"):
            parts = content.split(" ")
            if len(parts) >= 3:
                target_id = int(parts[1])
                job_name = parts[2]
                
                if target_id == self.player.id_:
                    self.job_data["assigned_job"] = job_name
                    logger.debug(f"Newcomer {self.player.id_} assigned job: {job_name}")
        
        # Elder location update
        elif content.startswith("elder_location"):
            parts = content.split(" ")
            if len(parts) >= 3:
                # would need triangulation to work :/
                self.job_data["elder_location"] = (int(parts[1]), int(parts[2]))
    
    async def on_job_start(self):
        """Initialize newcomer state"""
        await super().on_job_start()
        await self.player.broadcast("newcomer_joined")
    
    def get_job_info(self):
        """Return newcomer-specific job information"""
        info = super().get_job_info()
        info.update({
            "seeking_assignment": self.job_data["waiting_for_assignment"],
            "elder_id": self.job_data["elder_id"]
        })
        return info
