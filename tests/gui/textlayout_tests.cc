#include <gtest/gtest.h>
#include <cdroid.h>
#include <core/layout.h>
class TEXTLAYOUT:public testing::Test{
public:
};

TEST_F(TEXTLAYOUT,benchmark){
    App app;
    std::string testTextUTF8 =
        "Hello World! ﺎﻠﺳﻼﻣ ﻊﻠﻴﻜﻣ (Peace be upon you) ﻡﺮﺤﺑﺍ "
        "This is a test with multiple languages including "
        "Arabic: ﻡﺮﺤﺑﺍ ﺏﺎﻠﻋﺎﻠﻣ and Persian: ﺱﻼﻣ ﺪﻧیﺍ "
        "ﺶﻛﺭﺍً for testing. Thank you! ﻢﺘﺷکﺮﻣ "
        "Line breaking should work properly with complex scripts.";
    Layout ll(32,800);
    ll.setText(testTextUTF8);

    auto startTime = std::chrono::high_resolution_clock::now();
    for(int i=0;i<100;i++)
        ll.relayout(true);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    std::cout << "performLayout took: " << duration.count() << " microseconds" << std::endl;
    app.exec();
}
