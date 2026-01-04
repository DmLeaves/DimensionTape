#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFileDialog>
#include <QStandardPaths>
#include "StickerData.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showAndRaise();
    void forceClose();

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void createSticker(const StickerConfig &config);
    void loadStickerConfig();
    void saveStickerConfig();
    void editSticker(const QString &stickerId);
    void editStickerWithConfig(const QString &stickerId, const StickerConfig &config); // 新增信号
    void deleteSticker(const QString &stickerId);
    void exitRequested();
    void requestStickerConfigs();

public slots:
    void onStickerCreated(const StickerConfig &config);
    void onStickerDeleted(const QString &stickerId);
    void onStickerConfigChanged(const StickerConfig &config);
    void onStickerConfigsUpdated(const QList<StickerConfig> &configs);

private slots:
    void onCreateStickerClicked();
    void onDeleteStickerClicked();
    void onEditStickerClicked();
    void onStickerListSelectionChanged();
    void onBrowseImageClicked();
    void onAddEventClicked();
    void onRemoveEventClicked();
    void onEventTableItemChanged(QTableWidgetItem *item); // 修复参数
    void onApplyChangesClicked();
    void onLoadConfigClicked();
    void onSaveConfigClicked();
    void onExitClicked();

private:
    void setupUI();
    void setupStickerList();
    void setupStickerEditor();
    void setupBasicTab();
    void setupEventEditor();
    void setupMenuBar();
    void setupStatusBar();

    void updateStickerEditor(const StickerConfig &config);
    void clearStickerEditor();
    StickerConfig getStickerConfigFromEditor() const;
    void updateEventTable();
    void updateStickerList();

    // UI 组件
    QWidget *m_centralWidget;
    QSplitter *m_mainSplitter;

    // 贴纸列表区域
    QWidget *m_stickerListWidget;
    QListWidget *m_stickerList;
    QPushButton *m_createStickerBtn;
    QPushButton *m_deleteStickerBtn;
    QPushButton *m_editStickerBtn;

    // 贴纸编辑器
    QWidget *m_editorWidget;
    QTabWidget *m_editorTabs;
    QCheckBox *m_allowDragCheckBox;
    QCheckBox *m_clickThroughCheckBox;

    // 基本属性标签页
    QWidget *m_basicTab;
    QLineEdit *m_nameEdit;
    QLineEdit *m_imagePathEdit;
    QPushButton *m_browseImageBtn;
    QSpinBox *m_xSpinBox;
    QSpinBox *m_ySpinBox;
    QSpinBox *m_widthSpinBox;
    QSpinBox *m_heightSpinBox;
    QDoubleSpinBox *m_opacitySpinBox;
    QCheckBox *m_visibleCheckBox;
    QCheckBox *m_desktopModeCheckBox;

    // 事件编辑标签页
    QWidget *m_eventsTab;
    QTableWidget *m_eventTable;
    QPushButton *m_addEventBtn;
    QPushButton *m_removeEventBtn;
    QComboBox *m_triggerComboBox;
    QComboBox *m_eventTypeComboBox;
    QLineEdit *m_targetEdit;
    QLineEdit *m_parametersEdit;

    // 控制按钮
    QPushButton *m_applyChangesBtn;
    QPushButton *m_loadConfigBtn;
    QPushButton *m_saveConfigBtn;

    // 数据
    QList<StickerConfig> m_stickerConfigs;
    QString m_currentStickerId;
};

#endif // MAINWINDOW_H
