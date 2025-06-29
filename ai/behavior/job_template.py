from abc import ABC, abstractmethod
from typing import Dict, List, Optional, Final, final, Set
from dataclasses import dataclass
import random
import logging

from ..player import Player

from .newcomer import Newcomer
from .elder import Elder
from .basic import Basic
from .garbler import Garbler
from .collector import Collector

CRITICAL_FOOD_THOLD: Final[int] = 5

logger = logging.getLogger(__name__)

class JobTemplate(ABC):
    """Base class for all player jobs"""

    def __init__(self, player: Player):
        self.player = player
        self.job_name = self.__class__.__name__.lower()
        self.active = True
        self.job_data = {}

    @abstractmethod
    async def execute_behavior(self):
        """Main behavior loop for this job"""
        pass

    @abstractmethod
    def should_transition(self) -> Optional[str]:
        """Check if this job should transition to another job"""
        pass

    @abstractmethod
    async def on_message_received(self, direction: int, message):
        """Handle incoming broadcast messages"""
        pass

    async def on_job_start(self):
        """Called when transitioning TO this job"""
        logger.debug(f"Player {self.player.id_} starting job: {self.job_name}")

    async def on_job_end(self):
        """Called when transitioning FROM this job"""
        logger.debug(f"Player {self.player.id_} ending job: {self.job_name}")

    @final
    def get_job_info(self) -> Dict:
        """Return job-specific information for broadcasting"""
        return {
            "job": self.job_name,
            "level": self.player.level,
            "player_id": self.player.id_
        }

class JobManager:
    """Manages job transitions and coordination"""

    def __init__(self, player):
        self.player = player
        self.current_job: Optional[JobTemplate] = None
        self.job_history: List[str] = []
        self.known_players: Dict[int, Dict] = {} # { player_id: job_info }

    async def set_job(self, job_class):
        """Transition to a new job"""
        if self.current_job:
            await self.current_job.on_job_end()
            self.job_history.append(self.current_job.job_name)

        self.current_job = job_class(self.player)
        await self.current_job.on_job_start()
        
        job_info = self.current_job.get_job_info()
        await self.player.broadcast(f"job_update {job_info}")

    async def update_known_player(self, player_id: int, job_info: Dict):
        """Update information about another player"""
        self.known_players[player_id] = job_info

    def get_players_with_job(self, job_name: str) -> List[int]:
        """Get list of player IDs with specific job"""
        return [
            pid for pid, info in self.known_players.items() 
            if info.get("job") == job_name
        ]

    def has_elder(self) -> bool:
        """Check if there's currently an elder in the known players"""
        return len(self.get_players_with_job("elder")) > 0

    async def check_transitions(self):
        """Check if current job should transition"""
        if not self.current_job:
            return

        new_job_name = self.current_job.should_transition()
        if new_job_name:
            await self._handle_transition(new_job_name)

    async def _handle_transition(self, new_job_name: str):
        """Handle job transition logic"""
        job_classes = {
            "newcomer": Newcomer,
            "elder": Elder,
            "basic": Basic,
            "garbler": Garbler,
            "collector": Collector
        }
        
        if new_job_name in job_classes:
            await self.set_job(job_classes[new_job_name])

    async def execute_current_job(self):
        """Execute the current job's behavior"""
        if self.current_job:
            await self.current_job.execute_behavior()

    async def handle_message(self, direction: int, message):
        """Handle incoming messages for current job"""
        if self.current_job:
            await self.current_job.on_message_received(direction, message)
