import logging
from typing import Optional

from .job_template import *

logger = logging.getLogger(__name__)

class Collector(JobTemplate):
    """
    Collector job gathers resources and returns them to the depot (in that case,
    the Elder's tile). Supports efficient resource collection and delivery.
    """
    
    def __init__(self, player):
        super().__init__(player)
        self.job_data = {
            "target_resource": None,
            "carrying_capacity": 20, # theoretically, it does have infinite storage, but
            "carrying_load": 0,      # it needs to come to the Elder periodically
            "returning_to_depot": False,
            "elder_location": None,
        }
    
    async def execute_behavior(self):
        """Main collector behavior: gather and deliver resources."""
        
        if self.job_data["returning_to_depot"]:
            # return to depot with resources
            await self._return_to_depot()
        else:
            # gather resources until full capacity
            if self.job_data["carrying_load"] >= self.job_data["carrying_capacity"]:
                self.job_data["returning_to_depot"] = True
            else:
                await self._gather_resources()
    
    async def _gather_resources(self):
        """Search and collect resources."""
        
        await self.player.get_inventory()
        look_response = await self.player.look()
        tiles = self.player.handle_look_response(look_response)
        
        for tile in tiles[:3]:
            for resource in ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame", "food"]:
                if resource in tile:
                    if await self.player.take(resource):
                        self.job_data["carrying_load"] += 1
                        logger.debug(f"Collector {self.player.id_} took {resource}")
                        if self.job_data["carrying_load"] >= self.job_data["carrying_capacity"]:
                            break
            if self.job_data["carrying_load"] >= self.job_data["carrying_capacity"]:
                break
        
        if self.job_data["carrying_load"] < self.job_data["carrying_capacity"]:
            await self.player.random_movement()
    
    async def _return_to_depot(self):
        """Navigate to depot and deliver resources."""
        
        if self.job_data["elder_location"] is None:
            if self.job_data["elder_location"] is None:
                # unknown depot location, random move to search/wait
                await self.player.random_movement()
                return
        
        # TODO: goto Elder (coordinates) and put resources there
        await self.player.random_movement()
        # if self.job_data["carrying_load"] > 0:
        #     for resource, amount in self.player.resources.items():
        #         if amount > 0:
        #             await self.player.broadcast(f"resource_delivery {resource} {amount}")
        #             await self.player.set(resource)
        #             self.player.resources[resource] = 0
        #     self.job_data["carrying_load"] = 0
        #     self.job_data["returning_to_depot"] = False
    
    async def on_message_received(self, direction: int, message):
        """Handle messages relevant to collector."""
        content = message.content.strip()
        
        # Elder location update
        if content.startswith("elder_location"):
            parts = content.split(" ")
            if len(parts) >= 3:
                # would need coordinates
                self.job_data["elder_location"] = (int(parts[1]), int(parts[2]))
    
    async def on_job_start(self):
        """Initialize collector job state."""
        await super().on_job_start()
        logger.info(f"Collector {self.player.id_} started resource collection.")
    
    def should_transition(self) -> Optional[str]:
        """Decide whether to transition jobs."""
        if self.player.food_stock <= CRITICAL_FOOD_THOLD:
            return "basic"
        return None
    
    def get_job_info(self):
        """Return collector-specific info."""
        info = super().get_job_info()
        info.update({
            "carrying_load": self.job_data["carrying_load"],
            "carrying_capacity": self.job_data["carrying_capacity"],
            "returning_to_depot": self.job_data["returning_to_depot"],
            "elder_location": self.job_data["elder_location"],
        })
        return info