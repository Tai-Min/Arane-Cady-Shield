from time import sleep
from threading import Thread, Event
import logging
import pyaml
import subprocess
import os
import signal
from MCUInstrument import MCUInstrument

logger = logging.getLogger(__name__)


class Daemon:
    """Daemon for supporting MCU communication"""
    __MCU_MAX_RETRIES: int = 30

    __stop_evt: Event
    __mcu_err_evt: Event
    __heartbeat_thread: Thread

    __instrument: MCUInstrument
    __game_sel = -1

    def __init__(self) -> None:
        self.__stop_evt = Event()
        self.__mcu_err_evt = Event()
        self.__heartbeat_thread = Thread(target=self.__heartbeat_task)
        self.__process = None

        self.__num_players = 0
        self.__display_state = False
        self.__joy1_brightness = 0
        self.__joy2_brightness = 0

    def start(self, instrument_port: str, instrument_timeout, instrument_addr: int) -> None:
        """Start the daemon"""
        logger.info("Starting MCU daemon")

        self.stop()
        self.__stop_evt.clear()

        self.__instrument = MCUInstrument(
            instrument_port, instrument_timeout, instrument_addr)

        self.__heartbeat_thread.start()

        self.__general_task()

    def stop(self) -> None:
        """Stop the daemon"""
        if not self.__heartbeat_thread.is_alive():
            return

        logger.info("Stopping MCU daemon")

        self.__stop_evt.set()
        self.__heartbeat_thread.join()

    def __general_task(self) -> None:

        sleep(3)  # Wait for serial to open
        while not self.__stop_evt.is_set() and not self.__mcu_err_evt.is_set():
            sleep(1)

            shutdown_req = self.__instrument.get_shutdown_request()
            if shutdown_req:
                logger.info("Shutdown requested via MCU, shutting down")
                # TODO: shutdown in a nice way

            game_sel = self.__instrument.get_gamesel_value()
            if game_sel != self.__game_sel:
                self.__game_sel = game_sel
                logger.info("Game changed to %d", self.__game_sel)
                self.__load_game(self.__game_sel)


            if self.__num_players > 0:
                self.__instrument.set_joy1_state(True)
                if self.__num_players > 1:
                    self.__instrument.set_joy2_state(True)
                else:
                    self.__instrument.set_joy2_state(False)
            else:
                self.__instrument.set_joy1_state(False)

            self.__instrument.set_joy1_brightness(self.__joy1_brightness)
            self.__instrument.set_joy2_brightness(self.__joy2_brightness)

            self.__instrument.set_display_state(self.__display_state)

    def __heartbeat_task(self) -> None:
        mcu_retries = 0

        sleep(3)  # Wait for serial to open
        while not self.__stop_evt.is_set() and not self.__mcu_err_evt.is_set():
            sleep(1.5)

            logger.debug("Sending SBC heartbeat")
            self.__instrument.send_sbc_heartbeat()

            if not self.__instrument.check_mcu_heartbeat():
                mcu_retries += 1
                if mcu_retries >= self.__MCU_MAX_RETRIES:
                    logger.error("Couldn't communicate with MCU, aborting!")
                    self.__mcu_err_evt.set()
                continue

            logger.debug("Received valid heartbeat from MCU")
            mcu_retries = 0

    def __load_game(self, slot) -> bool:
        self.__instrument.set_display_state(False)

        sleep(2)

        self.__instrument.set_joy1_brightness(0)
        self.__instrument.set_joy2_brightness(0)
        self.__instrument.set_joy1_state(False)
        self.__instrument.set_joy2_state(False)

        # Unload previous slot
        if self.__process:
            os.killpg(os.getpgid(self.__process.pid), signal.SIGTERM)

        if slot < 0:
            logger.error("Invalid slot: %d", slot)
            return

        # Load slots.yaml
        try:
            with open("slots.yaml", "r", encoding="utf8") as f:
                config = pyaml.yaml.safe_load(f)

        except FileNotFoundError as e:
            logger.error("slots.yaml not found: %s", str(e))
            self.__game_sel = -1
            self.__display_state = False
            return

        # Use selected slot
        slot = "slot" + str(slot)
        try:
            name = str(config[slot]["name"])
            self.__num_players = int(config[slot]["players"])
            open_script = str(config[slot]["open_script"])
            exit_script = str(config[slot]["exit_script"])
            self.__joy1_brightness = int(config[slot]["joy1_brightness"])
            self.__joy2_brightness = int(config[slot]["joy2_brightness"])
        except Exception as e:
            logger.error("Processing yaml file failed: %s", str(e))
            self.__game_sel = -1
            self.__display_state = False
            return

        # Open process from selected slot
        logger.info("Opening %s", name)
        try:
            self.__process = subprocess.Popen(open_script, shell=True)
        except Exception as e:
            logger.error("Failed to load %s", slot)
            self.__game_sel = -1
            self.__display_state = False
            return
        logger.info("Opened %s", name)

        sleep(5)  # Let the game settle

        self.__display_state = True

        return True
