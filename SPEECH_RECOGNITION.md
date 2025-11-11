# Voice Keyboard - Speech Recognition Integration (ESP-IDF)

## Speech Recognition Options

### 1. Vosk API (Recommended for ESP32 C3)
**Pros:**
- Works completely offline
- Optimized for embedded systems
- Multiple language support including Russian
- Open source with ESP32 port
- Low memory footprint with optimized models

**Cons:**
- Limited vocabulary compared to cloud solutions
- Model storage requires careful memory management
- Recognition accuracy depends on model quality

**ESP-IDF Implementation:**
```cpp
#include "vosk_api.h"
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "VOSK";

class VoskRecognizer {
private:
    VoskModel *model;
    VoskRecognizer *recognizer;
    
public:
    esp_err_t init(const char* model_path) {
        // Mount SPIFFS for model storage
        esp_vfs_spiffs_conf_t conf = {
            .base_path = "/spiffs",
            .partition_label = NULL,
            .max_files = 5,
            .format_if_mount_failed = true
        };
        ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));
        
        // Initialize Vosk model
        model = vosk_model_new(model_path);
        if (!model) {
            ESP_LOGE(TAG, "Failed to load model");
            return ESP_FAIL;
        }
        
        recognizer = vosk_recognizer_new(model, 16000.0);
        if (!recognizer) {
            ESP_LOGE(TAG, "Failed to create recognizer");
            return ESP_FAIL;
        }
        
        ESP_LOGI(TAG, "Vosk recognizer initialized");
        return ESP_OK;
    }
    
    std::string process_audio(const int16_t* audio_data, size_t audio_size) {
        vosk_recognizer_accept_waveform(recognizer, audio_data, audio_size);
        const char *result = vosk_recognizer_final_result(recognizer);
        
        if (result && strlen(result) > 0) {
            ESP_LOGI(TAG, "Recognition result: %s", result);
            return std::string(result);
        }
        
        return "";
    }
};
```

### 2. OpenAI Whisper API (Online Alternative)
**Pros:**
- High accuracy for Russian language
- Large vocabulary
- Good with noisy environments

**Cons:**
- Requires internet connection
- API costs
- Higher latency
- Complex JSON handling in ESP-IDF

**ESP-IDF Implementation:**
```cpp
#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_log.h"
#include "base64.h"

class WhisperTranscriber {
private:
    char api_key[64];
    esp_http_client_handle_t client;
    
public:
    esp_err_t init(const char* key) {
        strncpy(api_key, key, sizeof(api_key) - 1);
        
        esp_http_client_config_t config = {
            .url = "https://api.openai.com/v1/audio/transcriptions",
            .method = HTTP_METHOD_POST,
            .timeout_ms = 30000,
            .event_handler = http_event_handler,
        };
        
        client = esp_http_client_init(&config);
        return client ? ESP_OK : ESP_FAIL;
    }
    
    esp_err_t transcribe(const uint8_t* audio_data, size_t audio_size, std::string& result) {
        // Create multipart form data
        char* boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
        char* wav_header = create_wav_header(audio_size);
        
        // Build request body
        std::string body = build_multipart_body(audio_data, audio_size, wav_header, boundary);
        
        // Set headers
        char auth_header[128];
        snprintf(auth_header, sizeof(auth_header), "Bearer %s", api_key);
        
        esp_http_client_set_header(client, "Authorization", auth_header);
        esp_http_client_set_header(client, "Content-Type", 
            "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");
        
        esp_http_client_set_post_field(client, body.c_str(), body.length());
        
        // Execute request
        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            int status_code = esp_http_client_get_status_code(client);
            if (status_code == 200) {
                // Parse JSON response
                result = parse_whisper_response();
            }
        }
        
        return err;
    }
};
```

## ESP-IDF Audio Processing Pipeline

### 1. I2S Audio Capture
```cpp
#include "driver/i2s.h"
#include "esp_log.h"

#define I2S_SAMPLE_RATE     16000
#define I2S_CHANNEL_FORMAT  I2S_CHANNEL_FMT_ONLY_LEFT
#define I2S_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT

class AudioCapture {
private:
    i2s_port_t i2s_num;
    int16_t* audio_buffer;
    size_t buffer_size;
    
public:
    esp_err_t init(i2s_port_t port, i2s_pin_config_t pin_config) {
        i2s_num = port;
        buffer_size = 1024 * 8; // 8KB buffer
        
        audio_buffer = (int16_t*) heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
        if (!audio_buffer) {
            return ESP_ERR_NO_MEM;
        }
        
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = I2S_SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 8,
            .dma_buf_len = 64,
            .use_apll = false,
            .tx_desc_auto_clear = false,
            .fixed_mclk = 0
        };
        
        esp_err_t ret = i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
        if (ret != ESP_OK) return ret;
        
        ret = i2s_set_pin(i2s_num, &pin_config);
        return ret;
    }
    
    esp_err_t capture_audio(int16_t** data, size_t* bytes_read) {
        return i2s_read(i2s_num, audio_buffer, buffer_size, bytes_read, portMAX_DELAY);
    }
};
```

### 2. Audio Preprocessing
```cpp
#include "esp_log.h"
#include <cmath>

class AudioProcessor {
private:
    static constexpr float HIGH_PASS_CUTOFF = 80.0f;
    static constexpr float SAMPLE_RATE = 16000.0f;
    static constexpr int FILTER_ORDER = 4;
    
    // High-pass filter coefficients
    float hp_coeffs[FILTER_ORDER + 1];
    float hp_states[FILTER_ORDER];
    
public:
    void init() {
        // Calculate high-pass filter coefficients
        float cutoff_rad = 2.0f * M_PI * HIGH_PASS_CUTOFF / SAMPLE_RATE;
        calculate_butterworth_coeffs(hp_coeffs, cutoff_rad, FILTER_ORDER);
        
        // Initialize filter states
        memset(hp_states, 0, sizeof(hp_states));
    }
    
    void preprocess_audio(int16_t* audio, size_t length) {
        // Convert to float for processing
        float* float_audio = (float*) malloc(length * sizeof(float));
        for (size_t i = 0; i < length; i++) {
            float_audio[i] = (float) audio[i] / 32768.0f;
        }
        
        // Apply high-pass filter
        apply_high_pass_filter(float_audio, length);
        
        // Apply automatic gain control
        apply_agc(float_audio, length);
        
        // Convert back to int16_t
        for (size_t i = 0; i < length; i++) {
            float sample = float_audio[i] * 32768.0f;
            audio[i] = (int16_t) std::clamp(sample, -32768.0f, 32767.0f);
        }
        
        free(float_audio);
    }
    
private:
    void apply_high_pass_filter(float* audio, size_t length) {
        for (size_t i = 0; i < length; i++) {
            float input = audio[i];
            float output = hp_coeffs[0] * input;
            
            for (int j = 1; j <= FILTER_ORDER; j++) {
                output += hp_coeffs[j] * hp_states[j - 1];
                if (j < FILTER_ORDER) {
                    hp_states[j - 1] = hp_states[j];
                }
            }
            
            hp_states[FILTER_ORDER - 1] = input;
            audio[i] = output;
        }
    }
    
    void apply_agc(float* audio, size_t length) {
        static constexpr float TARGET_RMS = 0.1f;
        static constexpr float ATTACK_TIME = 0.001f;
        static constexpr float RELEASE_TIME = 0.1f;
        
        float envelope = 0.0f;
        float gain = 1.0f;
        
        for (size_t i = 0; i < length; i++) {
            float sample = audio[i];
            float abs_sample = fabsf(sample);
            
            // Update envelope
            float time_constant = (abs_sample > envelope) ? ATTACK_TIME : RELEASE_TIME;
            float alpha = expf(-1.0f / (time_constant * SAMPLE_RATE));
            envelope = alpha * envelope + (1.0f - alpha) * abs_sample;
            
            // Calculate gain
            if (envelope > 0.001f) {
                float target_gain = TARGET_RMS / envelope;
                float gain_alpha = 0.001f;
                gain = gain_alpha * target_gain + (1.0f - gain_alpha) * gain;
                gain = std::clamp(gain, 0.1f, 10.0f);
            }
            
            audio[i] = sample * gain;
        }
    }
};
```

### 3. Voice Activity Detection
```cpp
class VoiceActivityDetector {
private:
    static constexpr float VAD_THRESHOLD = 0.01f;
    static constexpr int MIN_VOICE_FRAMES = 10;
    static constexpr int SILENCE_FRAMES_THRESHOLD = 20;
    
    int voice_frame_count;
    int silence_frame_count;
    bool is_speaking;
    
public:
    VoiceActivityDetector() : voice_frame_count(0), silence_frame_count(0), is_speaking(false) {}
    
    bool detect_voice_activity(const int16_t* audio, size_t length) {
        // Calculate RMS energy
        float sum = 0.0f;
        for (size_t i = 0; i < length; i++) {
            float sample = (float) audio[i] / 32768.0f;
            sum += sample * sample;
        }
        float rms = sqrtf(sum / length);
        
        bool voice_detected = rms > VAD_THRESHOLD;
        
        if (voice_detected) {
            voice_frame_count++;
            silence_frame_count = 0;
            
            if (!is_speaking && voice_frame_count >= MIN_VOICE_FRAMES) {
                is_speaking = true;
                ESP_LOGI("VAD", "Speech started");
            }
        } else {
            silence_frame_count++;
            voice_frame_count = 0;
            
            if (is_speaking && silence_frame_count >= SILENCE_FRAMES_THRESHOLD) {
                is_speaking = false;
                ESP_LOGI("VAD", "Speech ended");
                return true; // Speech segment complete
            }
        }
        
        return false;
    }
};
```

## ESP-IDF Memory Management

### ESP32 C3 Memory Constraints
- **SRAM**: 400KB total
- **DRAM**: ~300KB available for application
- **Flash**: 4MB (partitioned for app, data, SPIFFS)
- **PSRAM**: Not available on C3 SuperMini

### Memory Optimization Strategies
```cpp
// Use static allocation for critical buffers
static int16_t audio_buffer[4096] __attribute__((aligned(4)));
static uint8_t compressed_buffer[1024] __attribute__((aligned(4)));

// Use heap caps for DMA-capable memory
class MemoryManager {
public:
    static void* allocate_dma_buffer(size_t size) {
        return heap_caps_malloc(size, MALLOC_CAP_DMA);
    }
    
    static void* allocate_iram_buffer(size_t size) {
        return heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    }
    
    static void check_memory_usage() {
        ESP_LOGI("MEMORY", "Free heap: %d bytes", esp_get_free_heap_size());
        ESP_LOGI("MEMORY", "Min free heap: %d bytes", esp_get_minimum_free_heap_size());
        
        size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
        ESP_LOGI("MEMORY", "Largest free block: %d bytes", largest_block);
    }
};
```

### Model Storage Optimization
```cpp
// Partition scheme for model storage
#define MODEL_PARTITION_OFFSET   0x200000  // 2MB offset
#define MODEL_PARTITION_SIZE    0x200000  // 2MB for model

class ModelManager {
public:
    esp_err_t load_model_from_partition() {
        const esp_partition_t* model_partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA, "model");
            
        if (!model_partition) {
            ESP_LOGE("MODEL", "Model partition not found");
            return ESP_ERR_NOT_FOUND;
        }
        
        // Map model partition to memory
        const void* model_data = NULL;
        esp_err_t err = esp_partition_mmap(model_partition, 0, model_partition->size, 
                                        ESP_PARTITION_MMAP_DATA, &model_data);
        
        if (err != ESP_OK) {
            ESP_LOGE("MODEL", "Failed to map model partition");
            return err;
        }
        
        // Initialize Vosk with mapped model
        model = vosk_model_new((const char*)model_data);
        
        return ESP_OK;
    }
};
```

## ESP-IDF Configuration

### sdkconfig Recommendations
```bash
# Enable SPIFFS for model storage
CONFIG_SPIFFS=y
CONFIG_SPIFFS_MAX_PARTITIONS=1

# I2S Configuration
CONFIG_I2S_ENABLE=y
CONFIG_I2S_ISR_II=y

# Memory Optimization
CONFIG_HEAP_TRACING=y
CONFIG_HEAP_TRACING_STACK_DEPTH=10
CONFIG_HEAP_TASK_TRACKING=y

# Network Configuration (for online services)
CONFIG_ESP_HTTP_CLIENT_ENABLE=y
CONFIG_HTTPD_WS_SUPPORT=y

# Logging Configuration
CONFIG_LOG_DEFAULT_LEVEL_INFO=y
CONFIG_LOG_MAXIMUM_LEVEL=3
```

### CMakeLists.txt
```cmake
idf_component_register(SRCS
    "speech_recognition.cpp"
    "audio_capture.cpp"
    "audio_processor.cpp"
    "vad_detector.cpp"
    "memory_manager.cpp"
    "model_manager.cpp"
    INCLUDE_DIRS
    "."
    REQUIRES
    driver
    esp_http_client
    cJSON
    vfs
    spiffs
)
```

## Error Handling and Reliability

### Network Error Handling
```cpp
class NetworkManager {
private:
    static constexpr int MAX_RETRIES = 3;
    static constexpr int RETRY_DELAY_MS = 1000;
    
public:
    esp_err_t transcribe_with_retry(const uint8_t* audio, size_t size, 
                                std::string& result) {
        for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
            esp_err_t err = transcribe_audio(audio, size, result);
            
            if (err == ESP_OK) {
                return ESP_OK;
            }
            
            ESP_LOGW("NETWORK", "Transcription attempt %d failed: %s", 
                     attempt + 1, esp_err_to_name(err));
            
            if (attempt < MAX_RETRIES - 1) {
                vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS * (attempt + 1)));
            }
        }
        
        return ESP_ERR_TIMEOUT;
    }
    
    bool check_network_connectivity() {
        wifi_ap_record_t ap_info;
        esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
        return (err == ESP_OK && ap_info.rssi > -90);
    }
};
```

### Audio Quality Validation
```cpp
class AudioValidator {
public:
    bool validate_audio_quality(const int16_t* audio, size_t length) {
        // Check audio level
        float rms = calculate_rms(audio, length);
        if (rms < 0.001f) {
            ESP_LOGW("AUDIO", "Audio too quiet: RMS = %f", rms);
            return false;
        }
        
        // Check for clipping
        int clipped_samples = count_clipped_samples(audio, length);
        float clipping_ratio = (float)clipped_samples / length;
        if (clipping_ratio > 0.05f) {
            ESP_LOGW("AUDIO", "Too much clipping: %.1f%%", clipping_ratio * 100);
            return false;
        }
        
        // Check for DC offset
        float dc_offset = calculate_dc_offset(audio, length);
        if (fabsf(dc_offset) > 0.1f) {
            ESP_LOGW("AUDIO", "DC offset too high: %f", dc_offset);
            return false;
        }
        
        ESP_LOGI("AUDIO", "Audio quality OK: RMS=%f, Clipping=%.1f%%, DC=%f", 
                 rms, clipping_ratio * 100, dc_offset);
        return true;
    }
};
```

## Configuration Management

### NVS Configuration Storage
```cpp
#include "nvs_flash.h"
#include "nvs.h"

struct SpeechConfig {
    float sensitivity = 0.5f;
    int max_recording_time = 30000;
    char language[4] = "ru";
    bool enable_noise_reduction = true;
    bool enable_agc = true;
    float confidence_threshold = 0.7f;
    char api_key[64] = "";
};

class ConfigManager {
private:
    nvs_handle_t nvs_handle;
    static const char* NAMESPACE;
    
public:
    esp_err_t init() {
        esp_err_t err = nvs_open("voice_keyboard", NVS_READWRITE, &nvs_handle);
        return err;
    }
    
    esp_err_t load_config(SpeechConfig& config) {
        size_t required_size;
        esp_err_t err = nvs_get_blob(nvs_handle, "config", &config, &required_size);
        
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            // Use defaults and save
            return save_config(config);
        }
        
        return err;
    }
    
    esp_err_t save_config(const SpeechConfig& config) {
        return nvs_set_blob(nvs_handle, "config", &config, sizeof(config));
    }
    
    esp_err_t commit() {
        return nvs_commit(nvs_handle);
    }
};
```

## Development Workflow

### 1. ESP-IDF Project Setup
```bash
# Create new ESP-IDF project
idf.py create-project voice-keyboard
cd voice-keyboard

# Configure for ESP32-C3
idf.py menuconfig
# Select: ESP32-C3 -> ESP32C3 SuperMini

# Set up partitions
idf.py partition-table
# Add custom partition for model storage
```

### 2. Build and Flash
```bash
# Build project
idf.py build

# Flash to device
idf.py -p COM3 flash monitor

# Monitor output
idf.py monitor
```

### 3. Debugging
```bash
# Enable core dumps
idf.py menuconfig
# Component config -> ESP32-specific -> Core dump

# View memory usage
idf.py monitor
# Use heap tracing commands
```

This ESP-IDF specific implementation provides:
- Native ESP32 C3 support with proper memory management
- Optimized I2S audio capture
- Efficient voice activity detection
- Robust error handling and network management
- Configuration persistence with NVS
- Proper partitioning for model storage