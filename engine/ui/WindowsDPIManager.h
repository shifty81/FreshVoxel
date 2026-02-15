#pragma once

#ifdef _WIN32

namespace fresh
{

/**
 * @brief Manages High DPI awareness for Windows applications
 * 
 * Ensures proper scaling on high DPI displays (4K, etc.) on Windows 10/11.
 * Provides per-monitor DPI awareness for optimal visual quality.
 */
class WindowsDPIManager
{
public:
    /**
     * @brief DPI awareness modes
     */
    enum class DPIAwarenessMode
    {
        Unaware,              // System handles all scaling (blurry on high DPI)
        SystemAware,          // Application scales once for primary monitor
        PerMonitorAware,      // Application scales separately for each monitor (best)
        PerMonitorAwareV2     // Enhanced per-monitor with improved support (Windows 10 1703+)
    };

    WindowsDPIManager();
    ~WindowsDPIManager();

    /**
     * @brief Initialize DPI awareness
     * @param mode Desired DPI awareness mode
     * @return true if successful
     */
    bool initialize(DPIAwarenessMode mode = DPIAwarenessMode::PerMonitorAwareV2);

    /**
     * @brief Get DPI scale factor for a window
     * @param windowHandle Window handle (HWND)
     * @return DPI scale factor (1.0 = 96 DPI, 1.5 = 144 DPI, 2.0 = 192 DPI, etc.)
     */
    float getDPIScaleForWindow(void* windowHandle);

    /**
     * @brief Get system DPI
     * @return System DPI value (typically 96, 120, 144, 192, etc.)
     */
    unsigned int getSystemDPI();

    /**
     * @brief Convert logical pixels to physical pixels
     * @param logicalPixels Logical pixel value
     * @param scale DPI scale factor
     * @return Physical pixel value
     */
    static int logicalToPhysical(int logicalPixels, float scale);

    /**
     * @brief Convert physical pixels to logical pixels
     * @param physicalPixels Physical pixel value
     * @param scale DPI scale factor
     * @return Logical pixel value
     */
    static int physicalToLogical(int physicalPixels, float scale);

    /**
     * @brief Check if high DPI is supported
     * @return true if Windows 10 or later
     */
    static bool isHighDPISupported();

    /**
     * @brief Get current DPI awareness mode
     */
    DPIAwarenessMode getCurrentMode() const { return m_currentMode; }

    /**
     * @brief Check if initialized
     */
    bool isInitialized() const { return m_initialized; }

private:
    bool setDPIAwarenessContext(DPIAwarenessMode mode);
    bool setDPIAwareness(DPIAwarenessMode mode);
    
    bool m_initialized;
    DPIAwarenessMode m_currentMode;
    float m_systemDPIScale;
};

} // namespace fresh

#endif // _WIN32
