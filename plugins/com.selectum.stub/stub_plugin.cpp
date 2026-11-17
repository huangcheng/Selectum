#include "stub_plugin.h"

namespace selectum {

QString StubPlugin::id() const
{
    return QStringLiteral("com.selectum.stub");
}

Result StubPlugin::invoke(const Invocation &invocation)
{
    Result result;
    result.status = ResultStatus::Ok;
    result.message = QStringLiteral("stub:") + invocation.context.text;
    return result;
}

} // namespace selectum
