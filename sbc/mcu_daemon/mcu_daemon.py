import signal
import logging
from daemon_impl import Daemon

PORT = "/dev/serial0"
TIMEOUT = 0.5
INSTRUMENT_ADDR = 1

logging.basicConfig(filename='mcu_daemon.log', level=logging.DEBUG)
logging.getLogger().addHandler(logging.StreamHandler())

logger = logging.getLogger(__name__)

d = Daemon()

def on_terminate(signum, stack):
    """Catch SIGTERM and close daemon gracefully"""
    d.stop()
    logger.warning("Daemon stopped via SIGTERM")

if __name__ == "__main__":
    signal.signal(signal.SIGTERM, on_terminate)
    
    logger.info("Daemon starting")
    try:
        d.start(PORT, TIMEOUT, INSTRUMENT_ADDR)
    except KeyboardInterrupt:
        d.stop()
        logger.warning("Daemon stopped via keyboard interrupt")
