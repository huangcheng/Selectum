#include "ax_selection.h"

#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>

#include <QString>

namespace selectum {
namespace {

QString cfStringToQ(CFStringRef s)
{
    if (!s) {
        return {};
    }
    const CFIndex len = CFStringGetLength(s);
    QString out;
    out.resize(int(len));
    CFStringGetCharacters(s, CFRangeMake(0, len), reinterpret_cast<UniChar *>(out.data()));
    return out;
}

bool roleLooksEditable(CFStringRef role)
{
    if (!role) {
        return false;
    }
    return CFStringCompare(role, CFSTR("AXTextField"), 0) == kCFCompareEqualTo
        || CFStringCompare(role, CFSTR("AXTextArea"), 0) == kCFCompareEqualTo
        || CFStringCompare(role, CFSTR("AXComboBox"), 0) == kCFCompareEqualTo
        || CFStringCompare(role, kAXTextFieldRole, 0) == kCFCompareEqualTo
        || CFStringCompare(role, kAXTextAreaRole, 0) == kCFCompareEqualTo;
}

} // namespace

AccessibilityPermission queryAccessibilityPermission()
{
    NSDictionary *options = @{(__bridge id)kAXTrustedCheckOptionPrompt: @NO};
    const Boolean trusted = AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)options);
    return trusted ? AccessibilityPermission::Granted : AccessibilityPermission::Denied;
}

void requestAccessibilityPermissionPrompt()
{
    NSDictionary *options = @{(__bridge id)kAXTrustedCheckOptionPrompt: @YES};
    AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)options);
}

bool readFocusedSelection(SelectionContext *out)
{
    if (!out) {
        return false;
    }
    *out = SelectionContext{};
    out->trigger = Trigger::Selection;

    if (queryAccessibilityPermission() != AccessibilityPermission::Granted) {
        return false;
    }

    AXUIElementRef systemWide = AXUIElementCreateSystemWide();
    if (!systemWide) {
        return false;
    }

    AXUIElementRef focused = nullptr;
    const AXError focusErr =
        AXUIElementCopyAttributeValue(systemWide, kAXFocusedUIElementAttribute, (CFTypeRef *)&focused);
    CFRelease(systemWide);
    if (focusErr != kAXErrorSuccess || !focused) {
        return false;
    }

    CFTypeRef selectedTextValue = nullptr;
    const AXError textErr =
        AXUIElementCopyAttributeValue(focused, kAXSelectedTextAttribute, &selectedTextValue);
    if (textErr != kAXErrorSuccess || !selectedTextValue
        || CFGetTypeID(selectedTextValue) != CFStringGetTypeID()) {
        if (selectedTextValue) {
            CFRelease(selectedTextValue);
        }
        CFRelease(focused);
        return false;
    }

    out->text = cfStringToQ((CFStringRef)selectedTextValue);
    CFRelease(selectedTextValue);

    if (out->text.trimmed().isEmpty()) {
        CFRelease(focused);
        return false;
    }

    CFTypeRef roleValue = nullptr;
    if (AXUIElementCopyAttributeValue(focused, kAXRoleAttribute, &roleValue) == kAXErrorSuccess
        && roleValue && CFGetTypeID(roleValue) == CFStringGetTypeID()) {
        out->editable = roleLooksEditable((CFStringRef)roleValue);
        CFRelease(roleValue);
    }

    Boolean settable = false;
    if (AXUIElementIsAttributeSettable(focused, kAXValueAttribute, &settable) == kAXErrorSuccess
        && settable) {
        out->editable = true;
    }

    CFTypeRef rangeValue = nullptr;
    if (AXUIElementCopyAttributeValue(focused, kAXSelectedTextRangeAttribute, &rangeValue)
            == kAXErrorSuccess
        && rangeValue) {
        CFTypeRef boundsValue = nullptr;
        if (AXUIElementCopyParameterizedAttributeValue(focused,
                                                       kAXBoundsForRangeParameterizedAttribute,
                                                       rangeValue,
                                                       &boundsValue)
                == kAXErrorSuccess
            && boundsValue && CFGetTypeID(boundsValue) == AXValueGetTypeID()) {
            CGRect rect = CGRectZero;
            if (AXValueGetValue((AXValueRef)boundsValue, kAXValueTypeCGRect, &rect)) {
                out->screenRect.x = rect.origin.x;
                out->screenRect.y = rect.origin.y;
                out->screenRect.w = rect.size.width;
                out->screenRect.h = rect.size.height;
            }
            CFRelease(boundsValue);
        }
        CFRelease(rangeValue);
    }

    CFRelease(focused);

    @autoreleasepool {
        NSRunningApplication *front = [[NSWorkspace sharedWorkspace] frontmostApplication];
        if (front.bundleIdentifier) {
            out->appId = QString::fromNSString(front.bundleIdentifier);
        }
        if (front.localizedName) {
            out->title = QString::fromNSString(front.localizedName);
        }
    }

    return true;
}

} // namespace selectum
