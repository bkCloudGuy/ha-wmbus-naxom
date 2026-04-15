#include "meters_common_implementation.h"
namespace
{
    struct Driver : public virtual MeterCommonImplementation
    {
        Driver(MeterInfo &mi, DriverInfo &di);
    private:
        void processContent(Telegram *t);
    };
    static bool ok = registerDriver([](DriverInfo& di)
    {
        di.setName("apator_op04");
        di.setDefaultFields("name,id,total,timestamp");
        di.setMeterType(MeterType::WaterMeter);
        di.addLinkMode(LinkMode::T1);
        di.addDetection(MANUFACTURER_APA, 0x1A, 0x07);
        di.usesProcessContent();
        di.setConstructor([](MeterInfo& mi, DriverInfo& di)
        {
            return std::shared_ptr<Meter>(new Driver(mi, di));
        });
    });
    Driver::Driver(MeterInfo &mi, DriverInfo &di)
        : MeterCommonImplementation(mi, di)
    {
        addNumericField(
            "total",
            Quantity::Volume,
            DEFAULT_PRINT_PROPERTIES,
            "Total water consumption");
    }
    void Driver::processContent(Telegram *t)
    {
        t->dll_type = 0x07;
        std::vector<uchar> content;
        t->extractPayload(&content);
        
        warning("(apator_op04) content size: %zu\n", content.size());
        for (size_t i = 0; i + 5 < content.size(); i++)
        {
            warning("(apator_op04) byte[%zu]=0x%02x byte[%zu]=0x%02x\n", i, content[i], i+1, content[i+1]);
            if (content[i] == 0x04 && content[i+1] == 0x13)
            {
                uint32_t raw =
                    content[i+2] |
                    (content[i+3] << 8) |
                    (content[i+4] << 16) |
                    (content[i+5] << 24);
                double total_m3 = raw / 1000.0;
                warning("(apator_op04) FOUND! raw=%u total_m3=%f\n", raw, total_m3);
                setNumericValue("total", Unit::M3, total_m3);
                return;
            }
        }
        warning("(apator_op04) 0x04 0x13 NOT FOUND!\n");
    }
}
