"""MODBUS instrument for Cady shield"""
import logging
from threading import Lock
import minimalmodbus

logger = logging.getLogger(__name__)


class MCUInstrument:
    """MODBUS instrument for Cady shield"""

    __Q_SHUTDOWN_FLAG_ADDR: int = 0
    __Q_DISPLAY_STATE_ADDR: int = 1
    __Q_JOY1_ENA_FLAG_ADDR: int = 2
    __Q_JOY2_ENA_FLAG_ADDR: int = 3

    __I_SHUTDOWN_REQ_ADDR: int = 0

    __AQ_SBC_HB_CNTR_ADDR: int = 0
    __AQ_JOY1_LED_BRIGHTNESS_ADDR: int = 1
    __AQ_JOY2_LED_BRIGHTNESS_ADDR: int = 2

    __AI_MCU_HB_CNTR_ADDR: int = 0
    __AI_MCU_GAMESEL_ADDR: int = 1

    __READ_COIL: int = 1
    __READ_INPUT: int = 2
    __READ_HOLDING_REGISTER: int = 3
    __READ_INPUT_REGISTER: int = 4
    __FORCE_SINGLE_COIL: int = 5
    __PRESET_SINGLE_REGISTER: int = 6

    __client: minimalmodbus.Instrument
    __instrument_mtx: Lock

    __sbc_heartbeat_cntr: int = 0
    __mcu_heartbeat_cntr: int = 0

    def __init__(self, port: str, timeout: int, slave_addr: int) -> None:
        self.__instrument_mtx = Lock()
        self.__client = minimalmodbus.Instrument(port, slave_addr)
        self.__client.serial.timeout = timeout

    def send_sbc_heartbeat(self) -> bool:
        """Send heartbeat value to MCU"""
        logger.debug("Sending SBC heartbeat")
        with self.__instrument_mtx:
            try:
                self.__client.write_register(
                    self.__AQ_SBC_HB_CNTR_ADDR, self.__sbc_heartbeat_cntr,
                    functioncode=self.__PRESET_SINGLE_REGISTER)

                self.__sbc_heartbeat_cntr += 1
                logger.debug("SBC heartbeat sent")
                return True

            except IOError:
                logger.error("Failed to send SBC heartbeat")
                return False

    def check_mcu_heartbeat(self) -> bool:
        """Check MCU heartbeat.
           Returns False on error or when MCU disconnected, True otherwise
        """
        with self.__instrument_mtx:
            try:
                cntr = self.__client.read_register(
                    self.__AI_MCU_HB_CNTR_ADDR, functioncode=self.__READ_INPUT_REGISTER)

                if cntr != self.__mcu_heartbeat_cntr:
                    return True

                return False

            except IOError:
                return False

    def get_shutdown_request(self) -> bool:
        """Get shutdown flag from MCU"""
        with self.__instrument_mtx:
            try:
                return self.__client.read_bit(
                    self.__I_SHUTDOWN_REQ_ADDR, functioncode=self.__READ_INPUT)

            except IOError:
                return False

    def __write_bit(self, addr: int, val: bool) -> bool:
        with self.__instrument_mtx:
            try:
                self.__client.write_bit(
                    addr, val, functioncode=self.__FORCE_SINGLE_COIL)
                return True

            except IOError:
                return False

    def set_shutdown_flag(self) -> bool:
        """Notify MCU that SBC is shutting down"""
        flag = 1
        return self.__write_bit(self.__Q_SHUTDOWN_FLAG_ADDR, flag)

    def set_display_state(self, state: bool) -> bool:
        """Turn display on or off"""
        return self.__write_bit(self.__Q_DISPLAY_STATE_ADDR, state)

    def set_joy1_state(self, state: bool) -> bool:
        """Turn Joy 1 on or off"""
        return self.__write_bit(self.__Q_JOY1_ENA_FLAG_ADDR, state)

    def set_joy2_state(self, state: bool) -> bool:
        """Turn Joy 2 on or off"""
        return self.__write_bit(self.__Q_JOY2_ENA_FLAG_ADDR, state)

    def __write_register(self, addr: int, val: int) -> bool:
        """Set PWM value of Joy 1 LEDs"""
        with self.__instrument_mtx:
            try:
                self.__client.write_register(
                    addr, val, functioncode=self.__PRESET_SINGLE_REGISTER)
                return True

            except IOError:
                return False

    def set_joy1_brightness(self, val: int) -> bool:
        """Set PWM value of Joy 1 LEDs"""
        if val > 255:
            val = 255
        elif val < 0:
            val = 0

        self.__write_register(self.__AQ_JOY1_LED_BRIGHTNESS_ADDR, val)

    def set_joy2_brightness(self, val: int) -> bool:
        """Set PWM value of Joy 2 LEDs"""
        if val > 255:
            val = 255
        elif val < 0:
            val = 0

        self.__write_register(self.__AQ_JOY2_LED_BRIGHTNESS_ADDR, val)

    def get_gamesel_value(self) -> int:
        """Get game selector value, returns -1 on error, otherwise a valid gamesel value"""
        with self.__instrument_mtx:
            try:
                return self.__client.read_register(
                    self.__AI_MCU_GAMESEL_ADDR, functioncode=self.__READ_INPUT_REGISTER)
            except IOError:
                return -1
