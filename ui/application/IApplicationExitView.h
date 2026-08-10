#pragma once

namespace ui::application {

class IApplicationExitView {
public:
    virtual ~IApplicationExitView() = default;

    virtual void requestQuit() = 0;
};

} // namespace ui::application
