#ifndef INC_ALARMFIXTURE_H_
#define INC_ALARMFIXTURE_H_

#include "gtest.h"
#include "BaseModuleMock.h"

//include to register for Timer class
#include "ServiceMockForFuntionNeedToTest.h"


class AlarmTestFixture : public ::testing::Test
{
public:
    AlarmTestFixture(BaseModuleMock *mocks)
    {
        //Default
        _modulesMocks.reset(mocks);

        //Add mock module service
//        _FunctionUsedInTimer.reset(new ::testing::NiceMock<FunctionUsedInTimer>());
        //Add mock module service
        _oscillator.reset(new ::testing::NiceMock<OscillatorService>());
        _gpio.reset(new ::testing::NiceMock<GpioService>());

    }

    ~AlarmTestFixture(void)
    {
        //Default
        _modulesMocks.reset();

        //Add mock module service
//        _FunctionUsedInTimer.reset();
        _oscillator.reset();
        _gpio.reset();

    }

//    //Default
//    template<typename T>
//    static T& GetMock()
//    {
//        auto ptr = dynamic_cast<T*>(_modulesMocks.get());
//        if (ptr == nullptr)
//        {
//            auto errMsg = "The test does not provide mock of \"" + std::string(typeid(T).name()) + "\"";
//            throw std::exception(errMsg.c_str());
//        }
//        return *ptr;
//    }

    // Modules mocks - Default
    static std::unique_ptr<BaseModuleMock> _modulesMocks;

    // Add Services to test
//    static std::unique_ptr<FunctionUsedInTimer> _FunctionUsedInTimer;
    static std::unique_ptr<OscillatorService> _oscillator;
    static std::unique_ptr<GpioService> _gpio;

};

#endif /* INC_ALARMFIXTURE_H_ */
