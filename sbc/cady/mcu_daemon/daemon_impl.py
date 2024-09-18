"""Daemon that manages communication with MCU"""
from time import sleep
from threading import Thread, Event, Lock
import logging
import subprocess
import os
import yaml
from MCUInstrument import MCUInstrument

logger = logging.getLogger(__name__)


class Daemon:
    """Daemon for supporting MCU communication"""
    __MCU_MAX_RETRIES: int = 30

    __stop_evt: Event
    __mcu_err_evt: Event

    __heartbeat_thread: Thread
    __process_mtx: Lock

    __instrument: MCUInstrument
    __game_sel = -1
    __process = None

    def __init__(self) -> None:
        self.__stop_evt = Event()
        self.__mcu_err_evt = Event()
        self.__heartbeat_thread = Thread(target=self.__heartbeat_task)
        self.__process_mtx = Lock()

    def start(self, instrument_port: str, instrument_timeout: int, instrument_addr: int) -> None:
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

        self.__kill_process()

        logger.info("Stopping MCU daemon")

        self.__stop_evt.set()
        self.__heartbeat_thread.join()

    def notify_shutdown(self) -> None:
        """Notify MCU about system shutdown"""
        self.__instrument.set_shutdown_flag()

    def __general_task(self) -> None:

        sleep(3)  # Wait for serial to open

        self.__enable_joys(0)

        while not self.__stop_evt.is_set() and not self.__mcu_err_evt.is_set():
            sleep(1)

            shutdown_req = self.__instrument.get_shutdown_request()
            if shutdown_req:
                logger.info("Shutdown requested via MCU, shutting down")

                self.__kill_process()

                os.system("shutdown -h now")
                # script should exit on shutdown anyway and set some flags in MCU by doing so

            game_sel = self.__instrument.get_gamesel_value()
            if game_sel != self.__game_sel and game_sel >= 0:
                logger.info("Game slot changed to %d", game_sel)
                self.__game_sel = self.__load_game(game_sel)

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
                    # self.__mcu_err_evt.set()
                continue

            logger.debug("Received valid heartbeat from MCU")
            mcu_retries = 0

    def __kill_process(self) -> None:
        with self.__process_mtx:
            if self.__process:
                self.__process.kill()
                self.__process = None

    def __load_game(self, slot: int) -> int:
        self.__instrument.set_display_state(False)

        sleep(2)

        self.__enable_joys(0)

        # Unload previous slot
        self.__kill_process()

        sleep(1)

        if slot < 0:
            logger.error("Invalid slot: %d", slot)
            return - 1

        # Load slots.yaml
        try:
            with open("slots.yaml", "r", encoding="utf8") as f:
                config = yaml.safe_load(f)

        except FileNotFoundError as e:
            logger.error("slots.yaml not found: %s", str(e))
            self.__game_sel = -1
            return -1

        num_players = 0
        joy1_brightness = 0
        joy2_brightness = 0
        settle_time = 0.1

        # Use selected slot
        slot_str = "slot" + str(slot)
        try:
            name = str(config[slot_str]["name"])
            num_players = int(config[slot_str]["players"])
            open_script = str(config[slot_str]["open_script"])

            joy1_brightness = int(config[slot_str]["joy1_brightness"])
            joy2_brightness = int(config[slot_str]["joy2_brightness"])

            settle_time = int(config[slot_str]["settle_time"])

        except Exception as e:
            logger.error("Processing yaml file failed: %s", str(e))
            self.__game_sel = -1
            return -1

        # Open process from selected slot
        logger.info("Opening %s", name)
        try:
            with self.__process_mtx:
                self.__process = subprocess.Popen(open_script, shell=True)
        except Exception as e:
            logger.error("Failed to load %s", slot_str)
            self.__game_sel = -1
            return -1

        logger.info("Opened %s", name)

        sleep(settle_time)  # Let the game settle

        self.__enable_joys(num_players, joy1_brightness, joy2_brightness)
        self.__instrument.set_display_state(True)

        return slot

    def __enable_joys(self, num_players: int, joy1_brightness: int = 0, joy2_brightness: int = 0) -> None:
        """Set joys on/off and LED brightness"""
        if num_players > 0:
            self.__instrument.set_joy1_state(True)
            self.__instrument.set_joy1_brightness(joy1_brightness)
            if num_players > 1:
                self.__instrument.set_joy2_state(True)
                self.__instrument.set_joy2_brightness(joy2_brightness)
            else:
                self.__instrument.set_joy2_state(False)
                self.__instrument.set_joy2_brightness(0)
        else:
            self.__instrument.set_joy1_state(False)
            self.__instrument.set_joy1_brightness(0)
            self.__instrument.set_joy2_state(False)
            self.__instrument.set_joy2_brightness(0)
