import logging
from typing import Optional, Dict, List

from .job_template import *

logger = logging.getLogger(__name__)

class Elder(JobTemplate):
    """
    Leadership role - only one per civilization.
    Manages job assignments and hatches new players.
    """
    
    def __init__(self, player):
        super().__init__(player)
        self.job_data = {
            "managed_players": {},  # { player_id: assigned_job }
            "base_location": None,  # Where elder stations themselves
            "resource_depot": {     # Resources available at base
                "linemate": 0,
                "deraumere": 0,
                "sibur": 0,
                "mendiane": 0,
                "phiras": 0,
                "thystame": 0,
                "food": 0
            },
            "job_quotas": {     # Approximative percentage wanted for each job
                "collector": .30,
                "basic": .40,
                "garbler": 0 # only when enemy is detected
            },
            "current_assignments": {  # Track current job distribution
                "collector": [],
                "basic": [],
                "scout": [],
                "guardian": [],
                "garbler": []
            },
            "pending_newcomers": [], # Newcomers waiting for assignment
            "enemy_detected": False,
            "last_announcement": 0
        }
    
    async def execute_behavior(self):
        """Elder management behavior"""
        
        # Announce elder status periodically
        if self.player.counter % 20 == 0:
            await self.player.broadcast(f"elder_announce {self.player.id_}")
        
        # Process newcomers and assign jobs
        await self._process_newcomers()
        
        # Manage existing assignments
        await self._manage_assignments()
        
        # Collect resources at base location
        await self._manage_base()
        
        # Check for evolution opportunities
        await self._coordinate_evolution()
    
    async def _process_newcomers(self):
        """Assign jobs to waiting newcomers"""
        for newcomer_id in self.job_data["pending_newcomers"][:]:
            job = self._determine_best_job()
            if job:
                await self.player.broadcast(f"assign_job {newcomer_id} {job}")
                self.job_data["managed_players"][newcomer_id] = job
                self.job_data["current_assignments"][job].append(newcomer_id)
                self.job_data["pending_newcomers"].remove(newcomer_id)
                logger.debug(f"Elder assigned {job} to player {newcomer_id}")
    
    def _determine_best_job(self) -> Optional[str]:
        """Determine what job is most needed"""
        
        # Priority order based on current needs
        for job, quota in self.job_data["job_quotas"].items():
            current_count = len(self.job_data["current_assignments"][job])
            if current_count < quota:
                return job
        
        # If all quotas filled, default to collector
        return "collector"
    
    async def _manage_assignments(self):
        """Monitor and adjust job assignments"""
        
        # Remove dead/disconnected players
        active_players = set(self.player.job_manager.known_players.keys())
        for job, players in self.job_data["current_assignments"].items():
            self.job_data["current_assignments"][job] = [
                p for p in players if p in active_players
            ]
        
        # Adjust quotas based on situation
        if self.job_data["enemy_detected"]:
            self.job_data["job_quotas"]["garbler"] = .30
        
        # Request status updates from managed players
        if self.player.counter % 30 == 0:
            await self.player.broadcast("status_report_request")
    
    async def _manage_base(self):
        """Manage the base location and resources"""
        
        if not self.job_data["base_location"]:
            self.job_data["base_location"] = "current"
            await self.player.broadcast(f"elder_location 0 0") # TODO: use actual coordinates
        
        look_response = await self.player.look()
        tiles = self.player.handle_look_response(look_response)
        
        current_tile = tiles[0] if tiles else []
        for resource in ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame", "food"]:
            if resource in current_tile:
                await self.player.take(resource)
                if resource == "food":
                    self.job_data["resource_depot"]["food"] += 1
                else:
                    self.job_data["resource_depot"][resource] += 1
    
    async def _coordinate_evolution(self):
        """Coordinate team evolution attempts"""
        
        next_level = self.player.level + 1
        if str(next_level) in self.player.elevation_requirements:
            requirements = self.player.elevation_requirements[str(next_level)]
            
            total_resources = {}
            for resource in self.job_data["resource_depot"]:
                total_resources[resource] = (
                    self.job_data["resource_depot"][resource] + 
                    self.player.resources.get(resource, 0)
                )
            
            can_evolve = True
            for resource, needed in requirements.items():
                if resource == "players":
                    active_count = sum(len(players) for players in self.job_data["current_assignments"].values()) + 1
                    if active_count < needed:
                        can_evolve = False
                        break
                else:
                    if total_resources.get(resource, 0) < needed:
                        can_evolve = False
                        break
            
            if can_evolve:
                await self._initiate_team_evolution(requirements)
    
    async def _initiate_team_evolution(self, requirements):
        """Start a coordinated evolution attempt"""
        needed_players = requirements["players"]
        
        if self.evolve() != True:
            # call nearby players for evolution if couldn't evolve
            await self.player.broadcast(f"evolution_call {needed_players - self.player.get_current_tile_player_amount()}")

    def should_transition(self) -> Optional[str]:
        """Elder should rarely transition unless dying or major circumstances"""
        
        # only transition if health critical and no food
        if self.player.food_stock <= CRITICAL_FOOD_THOLD:
            # TODO: get food from depot
            if self.job_data["resource_depot"]["food"] > 0:
                self.job_data["resource_depot"]["food"] -= 1 # actually take it
                return None
            else:
                return "basic" # emergency!! step down and become basic to get food
        
        return None
    
    async def on_message_received(self, direction: int, message):
        """Handle incoming messages"""
        content = message.content.strip()
        sender_id = message.bot_id
        
        # newcomer seeking assignment
        if content.startswith("newcomer_seeking_assignment"):
            if sender_id not in self.job_data["pending_newcomers"]:
                self.job_data["pending_newcomers"].append(sender_id)
                logger.debug(f"Elder received newcomer {sender_id}")
        
        # resource delivery from collectors
        elif content.startswith("resource_delivery"):
            parts = content.split(" ")
            if len(parts) >= 3:
                resource = parts[1]
                amount = int(parts[2])
                self.job_data["resource_depot"][resource] += amount
                logger.debug(f"Elder received {amount} {resource} from player {sender_id}")
        
        elif content.startswith("enemy_detected"):
            self.job_data["enemy_detected"] = True
            logger.debug("Elder received enemy detection report")
    
    async def on_job_start(self):
        """Initialize elder role"""
        await super().on_job_start()
        await self.player.broadcast(f"elder_announce {self.player.id_}")
        logger.info(f"Player {self.player.id_} became Elder")
    
    async def on_job_end(self):
        """Clean up elder role"""
        await super().on_job_end()
        # Announce elder stepping down
        await self.player.broadcast("elder_stepping_down")
        logger.info(f"Player {self.player.id_} stepped down as Elder")
    
    def get_job_info(self):
        """Return elder-specific information"""
        info = super().get_job_info()
        info.update({
            "managed_players": len(self.job_data["managed_players"]),
            "base_location": self.job_data["base_location"],
            "resource_depot": self.job_data["resource_depot"]
        })
        return info
