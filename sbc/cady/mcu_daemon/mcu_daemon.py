"""MCU daemon"""
import signal
import sys
import logging
from daemon_impl import Daemon

PORT = "/dev/ttyAMA0"
TIMEOUT = 0.5
INSTRUMENT_ADDR = 1

logging.basicConfig(filename='mcu_daemon.log', level=logging.INFO)
logging.getLogger().addHandler(logging.StreamHandler())

logger = logging.getLogger(__name__)

d = Daemon()


def on_terminate(signum, stack) -> None:
    """Catch shutdown signals and close daemon gracefully"""
    d.notify_shutdown()
    d.stop()
    logger.warning("Daemon stopped via signal")


if __name__ == "__main__":
    signal.signal(signal.SIGTERM, on_terminate)

#    if sys.platform != "win32":
#        signal.signal(signal.SIGHUP, on_terminate)
#        signal.signal(signal.SIGCHLD, on_terminate)
#        signal.signal(signal.SIGALRM, on_terminate)

    logger.info("Daemon starting")
    try:
        d.start(PORT, TIMEOUT, INSTRUMENT_ADDR)
    except KeyboardInterrupt:
        d.stop()
        logger.warning("Daemon stopped via keyboard interrupt")
