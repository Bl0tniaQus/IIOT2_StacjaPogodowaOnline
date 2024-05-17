#ifndef DESKTOP_APP_H
#define DESKTOP_APP_H

#include <QMainWindow>
#include <QScopedPointer>

namespace Ui {
class Desktop_App;
}

class Desktop_App : public QMainWindow
{
    Q_OBJECT

public:
    explicit Desktop_App(QWidget *parent = nullptr);
    ~Desktop_App() override;

private:
    QScopedPointer<Ui::Desktop_App> m_ui;
};

#endif // DESKTOP_APP_H