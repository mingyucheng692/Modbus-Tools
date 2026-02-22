#pragma once

#include <QWidget>
#include <QList>

class QTableWidget;
class QPushButton;
class QEvent;

namespace ui::widgets {

struct QuickCommand {
    QString name;
    QString data;
    bool isHex; // true=Hex, false=Ascii
};

class QuickCommandWidget : public QWidget {
    Q_OBJECT

public:
    explicit QuickCommandWidget(QWidget *parent = nullptr);
    ~QuickCommandWidget() override;

    QList<QuickCommand> getCommands() const;
    void setCommands(const QList<QuickCommand>& commands);

signals:
    void sendRequested(const QByteArray& data);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onRemoveClicked();
    void onSendClicked();
    void onTableDoubleClicked(int row, int column);
    void onExportClicked();
    void onImportClicked();

private:
    void setupUi();
    void updateRow(int row, const QuickCommand& cmd);
    QByteArray parseData(const QString& data, bool isHex);
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    QTableWidget* table_ = nullptr;
    QPushButton* addBtn_ = nullptr;
    QPushButton* editBtn_ = nullptr;
    QPushButton* removeBtn_ = nullptr;
    QPushButton* sendBtn_ = nullptr;
    QPushButton* exportBtn_ = nullptr;
    QPushButton* importBtn_ = nullptr;
};

} // namespace ui::widgets
