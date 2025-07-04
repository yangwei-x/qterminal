#ifndef LAYERSHELLQT_STUB_H
#define LAYERSHELLQT_STUB_H

#include <QObject>
#include <QWindow>
#include <QMargins>

// Simple stub for LayerShellQt to allow compilation without the real library
namespace LayerShellQt {
    class Window : public QObject {
    public:
        enum Layer {
            LayerBackground,
            LayerBottom,
            LayerTop,
            LayerOverlay
        };
        
        enum Anchor {
            AnchorTop = 1,
            AnchorBottom = 2,
            AnchorLeft = 4,
            AnchorRight = 8
        };
        Q_DECLARE_FLAGS(Anchors, Anchor)
        
        enum KeyboardInteractivity {
            KeyboardInteractivityNone,
            KeyboardInteractivityExclusive,
            KeyboardInteractivityOnDemand
        };
        
        enum ScreenConfiguration {
            ScreenFromCompositor
        };
        
        static Window* get(QWindow* window) { Q_UNUSED(window); return nullptr; }
        
        void setLayer(Layer layer) { Q_UNUSED(layer); }
        void setKeyboardInteractivity(KeyboardInteractivity interactivity) { Q_UNUSED(interactivity); }
        void setAnchors(Anchors anchors) { Q_UNUSED(anchors); }
        void setScope(const QString& scope) { Q_UNUSED(scope); }
        void setScreenConfiguration(ScreenConfiguration config) { Q_UNUSED(config); }
        void setMargins(const QMargins& margins) { Q_UNUSED(margins); }
        Layer layer() const { return LayerTop; }
        Anchors anchors() const { return AnchorTop; }
    };
    
    Q_DECLARE_OPERATORS_FOR_FLAGS(Window::Anchors)
}

#endif // LAYERSHELLQT_STUB_H
