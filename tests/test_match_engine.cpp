#include <QtTest>

#include <core/match_engine.h>

using namespace selectum;

namespace {

PluginRecord makePlugin(const QString &id)
{
    PluginRecord p;
    p.id = id;
    p.enabled = true;
    return p;
}

SelectionContext makeCtx(const QString &text)
{
    SelectionContext ctx;
    ctx.text = text;
    ctx.appId = QStringLiteral("com.example.app");
    ctx.editable = false;
    ctx.trigger = Trigger::Selection;
    return ctx;
}

} // namespace

class TestMatchEngine : public QObject
{
    Q_OBJECT

private slots:
    void emptyPlugins_returnsEmpty();
    void minLength_filtersShortText();
    void editableOnly_requiresEditable();
    void appDeny_excludes();
    void appAllow_restricts();
    void disabledPlugin_excluded();
    void trigger_mustBeListed();
    void invalidRegex_doesNotThrow();
};

void TestMatchEngine::emptyPlugins_returnsEmpty()
{
    const QVector<PluginRecord> plugins;
    const auto matched = matchPlugins(plugins, makeCtx(QStringLiteral("hello")), Trigger::Selection);
    QCOMPARE(matched.size(), 0);
}

void TestMatchEngine::minLength_filtersShortText()
{
    auto plugin = makePlugin(QStringLiteral("com.selectum.minlen"));
    plugin.match.minLength = 5;

    SelectionContext shortCtx = makeCtx(QStringLiteral("hi"));
    SelectionContext longCtx = makeCtx(QStringLiteral("hello"));

    QCOMPARE(matchPlugins({plugin}, shortCtx, Trigger::Selection).size(), 0);
    QCOMPARE(matchPlugins({plugin}, longCtx, Trigger::Selection).size(), 1);
}

void TestMatchEngine::editableOnly_requiresEditable()
{
    auto plugin = makePlugin(QStringLiteral("com.selectum.editable"));
    plugin.match.editableOnly = true;

    SelectionContext nonEditable = makeCtx(QStringLiteral("hello"));
    nonEditable.editable = false;
    SelectionContext editable = makeCtx(QStringLiteral("hello"));
    editable.editable = true;

    QCOMPARE(matchPlugins({plugin}, nonEditable, Trigger::Selection).size(), 0);
    QCOMPARE(matchPlugins({plugin}, editable, Trigger::Selection).size(), 1);
}

void TestMatchEngine::appDeny_excludes()
{
    auto plugin = makePlugin(QStringLiteral("com.selectum.deny"));
    plugin.match.appDeny = QStringList{QStringLiteral("com.blocked.app")};

    SelectionContext denied = makeCtx(QStringLiteral("hello"));
    denied.appId = QStringLiteral("com.blocked.app");
    SelectionContext allowed = makeCtx(QStringLiteral("hello"));
    allowed.appId = QStringLiteral("com.ok.app");

    QCOMPARE(matchPlugins({plugin}, denied, Trigger::Selection).size(), 0);
    QCOMPARE(matchPlugins({plugin}, allowed, Trigger::Selection).size(), 1);
}

void TestMatchEngine::appAllow_restricts()
{
    auto plugin = makePlugin(QStringLiteral("com.selectum.allow"));
    plugin.match.appAllow = QStringList{QStringLiteral("com.allowed.app")};

    SelectionContext outside = makeCtx(QStringLiteral("hello"));
    outside.appId = QStringLiteral("com.other.app");
    SelectionContext inside = makeCtx(QStringLiteral("hello"));
    inside.appId = QStringLiteral("com.allowed.app");

    QCOMPARE(matchPlugins({plugin}, outside, Trigger::Selection).size(), 0);
    QCOMPARE(matchPlugins({plugin}, inside, Trigger::Selection).size(), 1);
}

void TestMatchEngine::disabledPlugin_excluded()
{
    auto plugin = makePlugin(QStringLiteral("com.selectum.off"));
    plugin.enabled = false;

    QCOMPARE(matchPlugins({plugin}, makeCtx(QStringLiteral("hello")), Trigger::Selection).size(), 0);
}

void TestMatchEngine::trigger_mustBeListed()
{
    auto plugin = makePlugin(QStringLiteral("com.selectum.trigger"));
    plugin.match.triggers = QStringList{QStringLiteral("selection"), QStringLiteral("palette")};

    QCOMPARE(matchPlugins({plugin}, makeCtx(QStringLiteral("hello")), Trigger::Selection).size(), 1);
    QCOMPARE(matchPlugins({plugin}, makeCtx(QStringLiteral("hello")), Trigger::Hotkey).size(), 0);
}

void TestMatchEngine::invalidRegex_doesNotThrow()
{
    auto plugin = makePlugin(QStringLiteral("com.selectum.badregex"));
    plugin.match.regex = QStringLiteral("(");

    QVector<PluginRecord> matched;
    try {
        matched = matchPlugins({plugin}, makeCtx(QStringLiteral("hello")), Trigger::Selection);
    } catch (...) {
        QFAIL("matchPlugins must not throw on invalid regex");
    }
    QCOMPARE(matched.size(), 0);
}

QTEST_MAIN(TestMatchEngine)
#include "test_match_engine.moc"
