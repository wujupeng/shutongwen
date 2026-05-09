#include "ime/language/LanguageModel.h"
#include "ime/language/NGramModel.h"

namespace ShuTongWen
{
    namespace Language
    {
        std::unique_ptr<LanguageModel> CreateLanguageModel()
        {
            return std::make_unique<NGramModel>();
        }
    }
}