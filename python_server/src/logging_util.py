import logging
import sys
from pathlib import Path

def setup_logging(log_level: str = "INFO", log_file: str = "dog_collar.log"):

    # Create logs directory if it doesn't exist
    Path("logs").mkdir(exist_ok=True)
    
    # Configure logging
    logging.basicConfig(
        level=getattr(logging, log_level.upper()),
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        handlers=[
            logging.FileHandler(f"logs/{log_file}"),
            logging.StreamHandler(sys.stdout)
        ]
    )

def get_logger(name: str):
    return logging.getLogger(name)