#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace ShuTongWen
{
    namespace OCR
    {
        struct OCRResult
        {
            std::wstring text;
            float confidence;
            int x;
            int y;
            int width;
            int height;

            OCRResult() : confidence(0.0f), x(0), y(0), width(0), height(0) {}
            OCRResult(const std::wstring& t, float c, int x_, int y_, int w, int h)
                : text(t), confidence(c), x(x_), y(y_), width(w), height(h) {}
        };

        struct OCRPageResult
        {
            std::vector<OCRResult> results;
            float averageConfidence;
            bool success;
            std::wstring errorMessage;

            OCRPageResult() : averageConfidence(0.0f), success(false) {}
        };

        enum class CaptureMode
        {
            FullScreen,
            Window,
            Region
        };

        class OCRService
        {
        public:
            ~OCRService() = default;

            static OCRService& Instance();

            bool Initialize(const std::wstring& modelPath = L"");
            void Uninitialize();

            bool IsInitialized() const { return m_initialized; }

            bool CaptureAndRecognize(CaptureMode mode, OCRPageResult& result);
            
            bool CaptureScreen(OCRPageResult& result);
            bool CaptureWindow(HWND hWnd, OCRPageResult& result);
            bool CaptureRegion(int x, int y, int width, int height, OCRPageResult& result);

            bool RecognizeFromImage(const std::vector<uint8_t>& imageData, OCRPageResult& result);
            bool RecognizeFromFile(const std::wstring& filePath, OCRPageResult& result);

            void SetModelPath(const std::wstring& path);
            std::wstring GetModelPath() const;

            void SetConfidenceThreshold(float threshold);
            float GetConfidenceThreshold() const;

            using OCRCallback = std::function<void(const OCRPageResult&)>;
            void SetOCRCallback(OCRCallback callback);

        private:
            OCRService();
            OCRService(const OCRService&) = delete;
            OCRService& operator=(const OCRService&) = delete;

            bool InitializeONNX();
            bool InitializeGraphicsCapture();

            std::vector<uint8_t> CaptureScreenInternal();
            std::vector<uint8_t> CaptureWindowInternal(HWND hWnd);
            std::vector<uint8_t> CaptureRegionInternal(int x, int y, int width, int height);

            OCRPageResult RunOCR(const std::vector<uint8_t>& imageData);

            bool m_initialized;
            std::wstring m_modelPath;
            float m_confidenceThreshold;
            
            void* m_onnxSession;
            void* m_captureManager;
            
            OCRCallback m_callback;
        };
    }
}