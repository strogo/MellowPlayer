#include <catch.hpp>
#include <MellowPlayer/Infrastructure/Plugins/PluginMetadata.hpp>
#include <UnitTests/Infrastructure/System/Fakes/FakeIniFileFactory.hpp>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

using namespace std;
using namespace MellowPlayer::Infrastructure;
using namespace MellowPlayer::Infrastructure::Tests;

SCENARIO("PluginMetadataTests")
{
    GIVEN("a PluginMetadata")
    {
        FakeIniFileFactory iniFileFactory;
        QString iniFilePath = "/path/to/plugin/metadata.ini";

        WHEN("loading metadata using fake data")
        {
            QMap<QString, QVariant> iniData;
            iniData["author"] = "authorName";
            iniData["author_website"] = "authorUrl";
            iniData["icon"] = "logo.svg";
            iniData["name"] = "pluginName";
            iniData["version"] = "pluginVersion";
            iniFileFactory.iniFileContents[iniFilePath] = iniData;

            std::shared_ptr<IIniFile> iniFile = iniFileFactory.create(move(iniFilePath));
            PluginMetadata pluginMetadata(iniFile);

            pluginMetadata.load();

            AND_THEN("values are correctly set")
            {
                REQUIRE(pluginMetadata.author() == "authorName");
                REQUIRE(pluginMetadata.authorUrl() == "authorUrl");
                REQUIRE(pluginMetadata.name() == "pluginName");
                REQUIRE(pluginMetadata.version() == "pluginVersion");
                REQUIRE(pluginMetadata.logo() == QFileInfo(QFileInfo(iniFilePath).dir(), "logo.svg").absoluteFilePath());
            }
        }
    }
}