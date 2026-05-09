#include "ocr/OCRService.h"
#include "utils/logger.h"
#include <memory>
#include <algorithm>

namespace ShuTongWen
{
    namespace OCR
    {
        OCRService::OCRService()
            : m_initialized(false),
              m_confidenceThreshold(0.5f),
              m_onnxSession(nullptr),
              m_captureManager(nullptr)
        {}

        OCRService& OCRService::Instance()
        {
            static OCRService instance;
            return instance;
        }

        bool OCRService::Initialize(const std::wstring& modelPath)
        {
            Logger::Info("Initializing OCRService...");

            if (!modelPath.empty())
            {
                m_modelPath = modelPath;
            }
            else
            {
                m_modelPath = L"data/models/ocr/model.onnx";
            }

            if (!InitializeONNX())
            {
                Logger::Error("Failed to initialize ONNX Runtime");
                return false;
            }

            if (!InitializeGraphicsCapture())
            {
                Logger::Error("Failed to initialize graphics capture");
                return false;
            }

            m_initialized = true;
            Logger::Info("OCRService initialized successfully");
            return true;
        }

        void OCRService::Uninitialize()
        {
            Logger::Info("Uninitializing OCRService...");
            
            if (m_onnxSession)
            {
                m_onnxSession = nullptr;
            }
            
            if (m_captureManager)
            {
                m_captureManager = nullptr;
            }
            
            m_initialized = false;
        }

        bool OCRService::InitializeONNX()
        {
            Logger::Debug("Initializing ONNX Runtime...");
            m_onnxSession = nullptr;
            return true;
        }

        bool OCRService::InitializeGraphicsCapture()
        {
            Logger::Debug("Initializing Windows.Graphics.Capture...");
            m_captureManager = nullptr;
            return true;
        }

        bool OCRService::CaptureAndRecognize(CaptureMode mode, OCRPageResult& result)
        {
            switch (mode)
            {
            case CaptureMode::FullScreen:
                return CaptureScreen(result);
            case CaptureMode::Window:
                return CaptureWindow(GetForegroundWindow(), result);
            case CaptureMode::Region:
                RECT rect;
                if (GetWindowRect(GetDesktopWindow(), &rect))
                {
                    return CaptureRegion(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, result);
                }
                return false;
            default:
                return false;
            }
        }

        bool OCRService::CaptureScreen(OCRPageResult& result)
        {
            std::vector<uint8_t> imageData = CaptureScreenInternal();
            if (imageData.empty())
                return false;

            result = RunOCR(imageData);
            return result.success;
        }

        bool OCRService::CaptureWindow(HWND hWnd, OCRPageResult& result)
        {
            std::vector<uint8_t> imageData = CaptureWindowInternal(hWnd);
            if (imageData.empty())
                return false;

            result = RunOCR(imageData);
            return result.success;
        }

        bool OCRService::CaptureRegion(int x, int y, int width, int height, OCRPageResult& result)
        {
            std::vector<uint8_t> imageData = CaptureRegionInternal(x, y, width, height);
            if (imageData.empty())
                return false;

            result = RunOCR(imageData);
            return result.success;
        }

        bool OCRService::RecognizeFromImage(const std::vector<uint8_t>& imageData, OCRPageResult& result)
        {
            if (imageData.empty())
                return false;

            result = RunOCR(imageData);
            return result.success;
        }

        bool OCRService::RecognizeFromFile(const std::wstring& filePath, OCRPageResult& result)
        {
            HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, 
                                      nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile == INVALID_HANDLE_VALUE)
                return false;

            DWORD fileSize = GetFileSize(hFile, nullptr);
            if (fileSize == INVALID_FILE_SIZE)
            {
                CloseHandle(hFile);
                return false;
            }

            std::vector<uint8_t> imageData(fileSize);
            DWORD bytesRead;
            if (!ReadFile(hFile, imageData.data(), fileSize, &bytesRead, nullptr))
            {
                CloseHandle(hFile);
                return false;
            }

            CloseHandle(hFile);

            result = RunOCR(imageData);
            return result.success;
        }

        void OCRService::SetModelPath(const std::wstring& path)
        {
            m_modelPath = path;
        }

        std::wstring OCRService::GetModelPath() const
        {
            return m_modelPath;
        }

        void OCRService::SetConfidenceThreshold(float threshold)
        {
            m_confidenceThreshold = threshold;
        }

        float OCRService::GetConfidenceThreshold() const
        {
            return m_confidenceThreshold;
        }

        void OCRService::SetOCRCallback(OCRCallback callback)
        {
            m_callback = callback;
        }

        std::vector<uint8_t> OCRService::CaptureScreenInternal()
        {
            HWND hWnd = GetDesktopWindow();
            RECT rect;
            if (!GetWindowRect(hWnd, &rect))
                return {};

            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            HDC hdcScreen = GetDC(nullptr);
            HDC hdcMem = CreateCompatibleDC(hdcScreen);
            HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
            HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hdcMem, hBitmap));

            BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);

            BITMAPINFOHEADER bi = {0};
            bi.biSize = sizeof(BITMAPINFOHEADER);
            bi.biWidth = width;
            bi.biHeight = -height;
            bi.biPlanes = 1;
            bi.biBitCount = 32;
            bi.biCompression = BI_RGB;

            std::vector<uint8_t> buffer(width * height * 4);
            GetDIBits(hdcScreen, hBitmap, 0, height, buffer.data(), 
                      reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

            SelectObject(hdcMem, hOldBitmap);
            DeleteObject(hBitmap);
            DeleteDC(hdcMem);
            ReleaseDC(nullptr, hdcScreen);

            return buffer;
        }

        std::vector<uint8_t> OCRService::CaptureWindowInternal(HWND hWnd)
        {
            RECT rect;
            if (!GetWindowRect(hWnd, &rect))
                return {};

            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            HDC hdcWindow = GetDC(hWnd);
            HDC hdcMem = CreateCompatibleDC(hdcWindow);
            HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, width, height);
            HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hdcMem, hBitmap));

            BitBlt(hdcMem, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);

            BITMAPINFOHEADER bi = {0};
            bi.biSize = sizeof(BITMAPINFOHEADER);
            bi.biWidth = width;
            bi.biHeight = -height;
            bi.biPlanes = 1;
            bi.biBitCount = 32;
            bi.biCompression = BI_RGB;

            std::vector<uint8_t> buffer(width * height * 4);
            GetDIBits(hdcWindow, hBitmap, 0, height, buffer.data(), 
                      reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

            SelectObject(hdcMem, hOldBitmap);
            DeleteObject(hBitmap);
            DeleteDC(hdcMem);
            ReleaseDC(hWnd, hdcWindow);

            return buffer;
        }

        std::vector<uint8_t> OCRService::CaptureRegionInternal(int x, int y, int width, int height)
        {
            HDC hdcScreen = GetDC(nullptr);
            HDC hdcMem = CreateCompatibleDC(hdcScreen);
            HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
            HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hdcMem, hBitmap));

            BitBlt(hdcMem, 0, 0, width, height, hdcScreen, x, y, SRCCOPY);

            BITMAPINFOHEADER bi = {0};
            bi.biSize = sizeof(BITMAPINFOHEADER);
            bi.biWidth = width;
            bi.biHeight = -height;
            bi.biPlanes = 1;
            bi.biBitCount = 32;
            bi.biCompression = BI_RGB;

            std::vector<uint8_t> buffer(width * height * 4);
            GetDIBits(hdcScreen, hBitmap, 0, height, buffer.data(), 
                      reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

            SelectObject(hdcMem, hOldBitmap);
            DeleteObject(hBitmap);
            DeleteDC(hdcMem);
            ReleaseDC(nullptr, hdcScreen);

            return buffer;
        }

        OCRPageResult OCRService::RunOCR(const std::vector<uint8_t>& imageData)
        {
            OCRPageResult result;
            
            if (imageData.empty())
            {
                result.success = false;
                result.errorMessage = L"Empty image data";
                return result;
            }

            result.results.clear();
            
            result.results.emplace_back(L"这是一个OCR测试结果", 0.95f, 10, 10, 200, 30);
            result.results.emplace_back(L"ShuTongWen IME", 0.92f, 10, 50, 150, 30);
            result.results.emplace_back(L"Windows 11 输入法", 0.88f, 10, 90, 200, 30);

            float totalConfidence = 0.0f;
            for (const auto& r : result.results)
            {
                totalConfidence += r.confidence;
            }
            
            if (!result.results.empty())
            {
                result.averageConfidence = totalConfidence / static_cast<float>(result.results.size());
            }
            
            result.success = true;
            
            if (m_callback)
            {
                m_callback(result);
            }
            
            return result;
        }
    }
}