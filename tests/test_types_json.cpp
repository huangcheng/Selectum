#include <QtTest>

#include <selectum/abi/json.h>
#include <selectum/abi/types.h>

using namespace selectum;

class TestTypesJson : public QObject
{
    Q_OBJECT

private slots:
    void roundTripSelectionContext();
    void roundTripInvocation();
    void roundTripResult();
};

void TestTypesJson::roundTripSelectionContext()
{
    SelectionContext ctx;
    ctx.text = QStringLiteral("hello world");
    ctx.appId = QStringLiteral("com.example.app");
    ctx.pageUrl = QStringLiteral("https://example.com/page");
    ctx.title = QStringLiteral("Example Page");
    ctx.editable = true;
    ctx.screenRect = ScreenRect{10.5, 20.25, 300.0, 40.0};
    ctx.trigger = Trigger::Selection;

    SelectionContext out;
    QVERIFY(fromJson(toJson(ctx), &out));
    QCOMPARE(out.text, ctx.text);
    QCOMPARE(out.appId, ctx.appId);
    QCOMPARE(out.pageUrl, ctx.pageUrl);
    QCOMPARE(out.title, ctx.title);
    QCOMPARE(out.editable, ctx.editable);
    QCOMPARE(out.screenRect.x, ctx.screenRect.x);
    QCOMPARE(out.screenRect.y, ctx.screenRect.y);
    QCOMPARE(out.screenRect.w, ctx.screenRect.w);
    QCOMPARE(out.screenRect.h, ctx.screenRect.h);
    QCOMPARE(out.trigger, ctx.trigger);
}

void TestTypesJson::roundTripInvocation()
{
    Invocation inv;
    inv.id = QStringLiteral("550e8400-e29b-41d4-a716-446655440000");
    inv.pluginId = QStringLiteral("com.selectum.stub");
    inv.context.text = QStringLiteral("selected");
    inv.context.appId = QStringLiteral("com.example.editor");
    inv.context.pageUrl = QString();
    inv.context.title = QStringLiteral("Untitled");
    inv.context.editable = false;
    inv.context.screenRect = ScreenRect{0.0, 0.0, 0.0, 0.0};
    inv.context.trigger = Trigger::Hotkey;
    inv.config.insert(QStringLiteral("endpoint"), QStringLiteral("https://api.example.com"));
    inv.config.insert(QStringLiteral("mode"), QStringLiteral("fast"));
    inv.secrets.insert(QStringLiteral("token"), QStringLiteral("secret-value"));
    inv.locale = QStringLiteral("zh_CN");

    Invocation out;
    QVERIFY(fromJson(toJson(inv), &out));
    QCOMPARE(out.id, inv.id);
    QCOMPARE(out.pluginId, inv.pluginId);
    QCOMPARE(out.context.text, inv.context.text);
    QCOMPARE(out.context.appId, inv.context.appId);
    QCOMPARE(out.context.pageUrl, inv.context.pageUrl);
    QCOMPARE(out.context.title, inv.context.title);
    QCOMPARE(out.context.editable, inv.context.editable);
    QCOMPARE(out.context.screenRect.x, inv.context.screenRect.x);
    QCOMPARE(out.context.screenRect.y, inv.context.screenRect.y);
    QCOMPARE(out.context.screenRect.w, inv.context.screenRect.w);
    QCOMPARE(out.context.screenRect.h, inv.context.screenRect.h);
    QCOMPARE(out.context.trigger, inv.context.trigger);
    QCOMPARE(out.config, inv.config);
    QCOMPARE(out.secrets, inv.secrets);
    QCOMPARE(out.locale, inv.locale);
}

void TestTypesJson::roundTripResult()
{
    Result result;
    result.status = ResultStatus::Error;
    result.message = QStringLiteral("something went wrong");
    result.openUrl = QStringLiteral("https://example.com/help");
    result.replaceText = QStringLiteral("replacement");

    Result out;
    QVERIFY(fromJson(toJson(result), &out));
    QCOMPARE(out.status, result.status);
    QCOMPARE(out.message, result.message);
    QCOMPARE(out.openUrl, result.openUrl);
    QCOMPARE(out.replaceText, result.replaceText);
}

QTEST_APPLESS_MAIN(TestTypesJson)
#include "test_types_json.moc"
