#pragma once

#include "Arduino.h"
#include "LinkedList.h"
#include "DMAChannel.h"
#include "AbstractBuffer.h"
#include "ArduinoJson.h"
#include "SdFat.h"

#define VARIABLE_TO_STRING(variable) (void(variable), #variable) // based on Stack Overflow https://stackoverflow.com/questions/3386861/converting-a-variable-name-to-a-string-in-c
                                                                 // comma operator runs the first operand AND discards result, and then evaluates the second operand and returns this value
                                                                 // this ensures that if the variable does not exist, the program will error

/**
 * @brief Structure to store the timing between various functions (filling buffers, main loop timing, etc.)
 * 
 * This allows for updating the timing information and storing it for later printing. Otherwise, attempting to
 * print the differences in between calls while they happen could result in spending excessive time printing to the
 * output buffer.
 */
struct TimingMetadata {
    volatile size_t counter; /**< How many times was this structure updated */
    volatile uint32_t min_diff; /**< The minimum difference in times between updates */
    volatile uint32_t max_diff; /**< The maximum difference in times between updates */
    volatile float avg_diff; /**< The average difference in times between updates */
    volatile uint32_t prior_time; /**< Needed to store the previous time of the trigger from which the difference can be calculated */
    volatile uint32_t first_trigger; /**< Time since program start of the first update */
    volatile bool was_triggered; /**< True if this was ever updated, otherwise false */
};
using TimingData = TimingMetadata;

/**
 * @brief Structure hold for how long sampling lasted
 * 
 */
struct SamplingMetadata {
    uint32_t start_time; /**< Milliseconds since program start the sampling was begun */
    uint32_t end_time;   /**< Milliseconds since program start the sampling was ended */
};

/**
 * @brief Structure to hold a timer and a delta time to use for printing variables every so often
 * 
 * If variables have to be printed during active running of the program (and would otherwise overwhelm the serial output if printed in loop()),
 * this structure is used to initialize a timer and a dt for time in between printing the output.
 * See Logger::print_timed_variables().
 */
struct PrintTimer {
    elapsedMillis timer;
    uint32_t dt;
};
using PrintTiming = PrintTimer;

/**
 * @brief Implements assorted printing of variables in various forms
 * 
 * For the code structure, the protected methods are responsible for just the printing of the variable itself.
 * The public methods include a ETAG_LOG_LEVEL (defined at compile time: teensy40, then ETAG_LOG_LEVEL is 0: OFF, if
 * logger, then ETAG_LOG_LEVEL is 1: ON). See .platformio file for references to teensy40 and logger. Thus, the public
 * methods will only print to the log only if the ETAG_LOG_LEVEL is 1.
 * 
 * For the logging scheme to work, -D USB_DUAL_SERIAL must be enabled in compile options, and SerialUSB1 used as the Stream. Data
 * cannot be streamed over Serial as Serial is used for USB communication with the GUI to interface with the tag.
 * Bluetooth is not meant in its current state to handle logging either, as the GUI itself cannot differentiate
 * between the commands from the tag and logging data if they are sent over the same line. Instead, two different
 * ports are used for data and logging.
 * 
 */
class VarPrinter {
    public:
        /**
         * @brief Construct a new Var Printer object
         * 
         * @param comms Pointer to a Stream (or something that inherited Stream) to be used to send the logging data to
         */
        VarPrinter(Stream* comms) : _serial(comms) {};

        /**
         * @brief Prints an array
         * 
         * Array will be printed in the following form:\n
         * `var_name: [array]`\n
         * 
         * @tparam T Must be a typename recognized by println (note that uint64_t will likely not work, but other types like size_t, unsigned char, int16_t should all work)
         * @param var_name The name to call the printed variable
         * @param head Pointer to the head of the array to print
         * @param len The length of the array to print
         */
        template <typename T>
        void print_array(String var_name, T* head, size_t len) {
            if (ETAG_LOG_LEVEL) {
                _serial->println(var_name + ": ");
                print_array(head, len);
            }
        }

        /**
         * @brief Print a single variable
         * 
         * Variable will be printed in the following form:\n
         * `var_name: var`\n
         * 
         * @tparam T Must be a typename recognized by print (note that uint64_t will work due to overload)
         * @param var_name The name to call the printed variable
         * @param var The variable value to print
         */
        template <typename T>
        void print_variable(String var_name, T var) {
            if (ETAG_LOG_LEVEL) {
                _serial->print(var_name + ": "); print_variable(var);
            }
        }

        /**
         * @brief Prints a variable along with an associated time
         * 
         * Variable will be printed in the following form:\n
         * `var_name: var   time`\n
         * 
         * The time is from device start (Teensy turning on, not since sampling start)
         * 
         * @tparam T Must be a typename recognized by print (64 byte variables will likely not work)
         * @param var_name The name to call the printed variable
         * @param var The variable value to print
         */
        template <typename T>
        void print_variable_with_time(String var_name, T var) {
            if (ETAG_LOG_LEVEL) {
                _serial->print(var_name + ": ");
                print_variable_with_time(var);
            }
        }

        /**
         * @brief Prints the TimingMetadata
         * 
         * Will be printed in the following form:\n 
         * `
         * var_name:
         * Num triggers:   value\n
         * Min diff:       value\n
         * Max diff:       value\n
         * Avg diff:       value\n
         * First trigger:  value\n
         * `
         * @param var_name The name to call the printed variable
         * @param var The TimingMetadata object to print
         */
        void print_timing_metadata(String var_name, TimingData* var) {
            if (ETAG_LOG_LEVEL) {
                _serial->println(var_name + ": "); print_timing_metadata(var);
            }
        }

        /**
         * @brief Prints a message
         * 
         * Will be printed in the following form:\n 
         * `message string`
         * 
         * @param message String to print to the logging output
         */
        void print_message(String message) {
            if (ETAG_LOG_LEVEL) {
                _serial->println(message);
            }
        };

        /**
         * @brief Prints DMA Settings (like the ones used in ADCBuffer)
         * 
         * Will be printed in the following form:\n
         * `SADDR:%x SOFF:%d ATTR:%x NBYTES:%x SLAST:%d DADDR:%x DOFF: %d CITER:%x DLASTSGA:%x CSR:%x BITER:%x`,
         * 
         * where SADDR, SOFF, ATTR, NBYTES, SLAST, DADDR, DOFF, CITER, DLASTSGA, CSR, and BITER refer to the
         * DMA registers (see the i.MX RT1060 Processor Reference Manual, Chapter 5 for details)
         * 
         * The formatting follows printf conventions, namely:
         * - `%x`: Unsiged hexadecimal integer
         * - `%d`: Signed decimal integer
         *
         * @param name The name to call the printed DMA settings
         * @param dmabc DMABaseClass pointer which stores the DMA settings
         */
        void print_DMA_settings(String name, DMABaseClass* dmabc) {
            if (ETAG_LOG_LEVEL) {
                _serial->println(name);
            }
        };

        /**
         * @brief Prints the header bytes of buffers stored in classes that inherit from AbstractBuffer (e.g. EEGBuffer, ADCBuffer, IMUBuffer)
         * 
         * Will be printed in the following form:\n
         * `var_name`
         * `==========`
         * `ID: %d Count: %d Time: %lu`
         * 
         * The formatting follows printf conventions, namely:
         * - `%d`: signed decimal integer
         * - `%lu`: unsigned long
         * 
         * @tparam T Any primeary data type
         * @param var_name The name to call the printed variable
         * @param buffers Pointer to the buffers object, an array of BufferHelper pointers
         * @param len Length of the array of BufferHelper pointers
         */
        template <typename T>
        void print_buffer_headers(String var_name, volatile BufferHelper<T>* buffers, size_t len) {
            if (ETAG_LOG_LEVEL) {
                _serial->println(var_name);
                _serial->println("==========");
                for (size_t i = 0; i < len; i++) {
                    _serial->printf("Loc: %p ID: %d Count: %d Time: %lu\n", (void *) buffers[i].id_ptr,
                                                                            *(buffers[i].id_ptr),
                                                                            *(buffers[i].count_ptr),
                                                                            *(buffers[i].time_ptr));
                }
            }
        }

        /**
         * @brief Serialize a JsonDocument over the serial
         * 
         * Note that the internal _serial is a Stream*, while serializeJsonPretty this function calls must instead be a Stream,
         * and thus _serial must be derefrenced in the function call. A new line must be printed as well as there is no trailing
         * new line after serializeJsonPretty.
         * 
         * For more information about ArduinoJson which is used for JSON Serialization, see the<a href=https://arduinojson.org/>Arduino JSON documentation</a>.
         * 
         * @param doc 
         */
        void print_json(const JsonDocument& doc) {
            if (ETAG_LOG_LEVEL) {
                serializeJsonPretty(doc, *_serial); _serial->print('\n');
            }
        }

        /**
         * @brief Print a variable with at least a minimum period in between prints
         * 
         * This ensures that the serial output does not get overwhelemed by repeated calls to print very often.
         * This function is most useful for printing a variable in the main loop(). The other functions are fine
         * to use for functions that are called more rarely.
         * 
         * @tparam T 
         * @param print_timer 
         * @param var_name 
         * @param var 
         */
        template <typename T>
        void print_timed_variables(PrintTiming& print_timer, String var_name, T var) {
            if (ETAG_LOG_LEVEL) {
                if (print_timer.timer > print_timer.dt) {
                    print_variable(var_name, var);
                    print_timer.timer = 0;
                }
            }
        }

        /**
         * @brief Prints the memory address of an object
         * 
         * @param var_name The name to call the printed memory address
         * @param var The memory address to print (as a void*)
         */
        void print_memory(String var_name, void* var) {
            if (ETAG_LOG_LEVEL) {
                _serial->print(var_name + ": ");_serial->printf("%p\n", var);
            }
        }

        /**
         * @brief Prints out the SD card error code
         * 
         * @param sd SdFS object which to query for SD error
         */
        void print_SD_error(SdFs& sd) {
            if (ETAG_LOG_LEVEL) {
                uint8_t error_code = sd.sdErrorCode();
                printSdErrorSymbol(_serial, error_code); _serial->print("\n"); //function from SD-Fat library
                printSdErrorText(_serial, error_code); _serial->print("\n");  //function from SD-Fat library
            }
        }

    protected:
        Stream* _serial; /**< Pointer to Stream which is where these values will be printed to */

        template <typename T>
        void print_array(T* head, size_t len) {
            _serial->print("[");
            for (size_t i = 0; i < len; i++)
            {
                _serial->print(head[i]); _serial->print(", ");
            }
            _serial->println("]");
        }

        template <class T>
        void print_variable(T var) {
            _serial->println(var);
        }

        void print_variable(uint64_t var) {
            _serial->printf("%llu\n", var);  // %llu -> unsigned long long
        }

        template <typename T>
        void print_variable_with_time(T var) {
            _serial->print(millis()); _serial->print("  "); print_variable(var);
        }

        void print_timing_metadata(TimingData* metadata) {
            _serial->print("Num triggers:   ");  _serial->println(metadata->counter);
            _serial->print("Min diff:       ");  _serial->println(metadata->min_diff);
            _serial->print("Max diff:       ");  _serial->println(metadata->max_diff);
            _serial->print("Avg diff:       ");  _serial->println(metadata->avg_diff);
            _serial->print("First trigger:  ");  _serial->println(metadata->first_trigger);
        }

        void print_sampling_metadata(SamplingMetadata* metadata) {
            _serial->print("Sampling start: "); _serial->println(metadata->start_time);
            _serial->print("Sampling end:   "); _serial->println(metadata->end_time);
        }

        void print_DMA_settings(DMABaseClass* dmabc) {
	        _serial->printf("%x %x:\n", (uint32_t)dmabc, (uint32_t)dmabc->TCD);

	        _serial->printf("SADDR:%x SOFF:%d ATTR:%x NBYTES:%x SLAST:%d DADDR:%x DOFF: %d CITER:%x DLASTSGA:%x CSR:%x BITER:%x\n", (uint32_t)dmabc->TCD->SADDR,
		                    dmabc->TCD->SOFF, dmabc->TCD->ATTR, dmabc->TCD->NBYTES, dmabc->TCD->SLAST, (uint32_t)dmabc->TCD->DADDR, 
		                    dmabc->TCD->DOFF, dmabc->TCD->CITER, dmabc->TCD->DLASTSGA, dmabc->TCD->CSR, dmabc->TCD->BITER);
        }       
};

/**
 * @brief The main logging class responsible for logging
 * 
 * This classes sets up the LinkedLists used for storing and then printing the TimingMetadata and other updates
 * of structures for logging. Since this class inherits the public interface of VarPrinter, this Logger is the one
 * that calls the printing of variables.
 * 
 * \todo Would have been more efficient to make VarPrinter have the protected methods become the public methods,
 * and for the public methods of VarPrinter to be the methods contained in Logger. Thus, there is a seperate class
 * that prints variables, and then a Logger that inherits the print methods and wraps them in Logging level to be used
 * for logging. Not a critical change, but one that would improve the format of the code.
 */
class Logger: public VarPrinter {
    public:
        /**
         * @brief Construct a new Logger object
         * 
         * A Serial output must be passed to the Logger
         * 
         * @param serial Stream to which the Logging data will be written
         */
        Logger(Stream* serial) : VarPrinter(serial) {};

        /**
         * @brief Adds the TimingData structures to the Logger
         * 
         * These will be added to a linked list that stores the location of all the TimingData structures.
         * This way, logging the timing metadata simply needs to iterate through the linked list to print them all out.
         * Thus, only the setup code gets a little messier (the TimingMetadata is added then to the logger one at a time),
         * but the rest of the code stays clean.
         * 
         * @param data Pointer to the TimingData structure to add to the linked list
         * @param name The name to call the TimingData structure
         */
        void assign_metadata(TimingData* data, String name);

        /**
         * @brief Updates the members of TimingData
         * 
         * This function when called will get the time the function was called and update all the fields of TimingData.
         * See TimingMetadata for what these fields are.
         * 
         * @param data The TimingData to update
         */
        void update_timing_data(TimingData& data);

        /**
         * @brief Prints the timing metadata
         * 
         * Loops over the linked list, printing the timing metadata. For format, see VarPrinter::print_timing_metadata
         * 
         */
        void log_timing_metadata();

        /**
         * @brief Loops over the linked list, resetting to defaults the TimingMetadata structure.
         * 
         * Every variable is set to either 0 or false, depending on type.
         * 
         */
        void reset_metadata();

        /**
         * @brief Get the length of the linked list
         * 
         * @return size_t The length of the linked list
         */
        size_t get_length() { return metadata_names.get_length(); }

    private:
        void get_average(TimingData* metadata); /**< Calculates the average for the timing metadata, called in log_timing_metadata() */

        LinkedList<TimingData*> timing_list; /**< Linked list of the timing data */
        LinkedList<String> metadata_names; /**< Linked list of the corresponding names of the timing data */
};