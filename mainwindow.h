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
#include <QHeaderView>
#include <QFileDialog>
#include <QStandardPaths>
#include "StickerData.h"
#include "windowrecognitionservice.h"

class EventEditorPanel;

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
    void lockFollowTarget(const QString &stickerId, qulonglong windowHandle);
    void unlockFollowTarget(const QString &stickerId);
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
    void onContentTypeChanged(int index);
    void onBrowseImageClicked();
    void onBrowseLive2DModelClicked();
    void onBrowseLive2DRuntimeClicked();
    void onEditorValueChanged();
    void onApplyChangesClicked();
    void onCancelChangesClicked();
    void onLoadConfigClicked();
    void onSaveConfigClicked();
    void onExitClicked();
    void onRefreshWindowsClicked();
    void onLockWindowClicked();
    void onBatchRefreshWindowsClicked();
    void onBatchLockWindowClicked();
    void onSuggestFollowFilterClicked();
    void onFollowModeToggled(bool enabled);
    void onFollowBatchModeToggled(bool enabled);
    void onEventsChanged(const QList<StickerEvent> &events);
    void onEventStatusMessage(const QString &message, int timeoutMs);

private:
    void setupUI();
    void setupStickerList();
    void setupStickerEditor();
    void setupBasicTab();
    void setupEventEditor();
    void setupMenuBar();
    void setupStatusBar();
    void connectEditorSignals();
    void beginEditSession();
    void applyPreviewIfEditing();
    void cancelPendingEdits();
    void refreshWindowList();
    void updateFollowModeUi();
    void updateContentTypeUi(StickerContentType type);
    bool isSingleFollowLocked() const;
    bool isBatchFollowLocked() const;
    QString buildFollowFilterValue(const WindowInfo &info) const;
    bool applyFollowFilterSuggestion(qulonglong handle, bool force, bool warnOnEmpty);
    void populateWindowCombo(QComboBox *combo);

    void updateStickerEditor(const StickerConfig &config);
    void clearStickerEditor();
    StickerConfig getStickerConfigFromEditor() const;
    void updateStickerList();
    int findConfigIndex(const QString &stickerId) const;

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
    QComboBox *m_contentTypeComboBox;
    QLineEdit *m_imagePathEdit;
    QPushButton *m_browseImageBtn;
    QGroupBox *m_live2dGroup;
    QLineEdit *m_live2dModelPathEdit;
    QPushButton *m_browseLive2DModelBtn;
    QLineEdit *m_live2dRuntimeRootEdit;
    QPushButton *m_browseLive2DRuntimeBtn;
    QLineEdit *m_live2dShaderProfileEdit;
    QSpinBox *m_xSpinBox;
    QSpinBox *m_ySpinBox;
    QSpinBox *m_widthSpinBox;
    QSpinBox *m_heightSpinBox;
    QDoubleSpinBox *m_opacitySpinBox;
    QCheckBox *m_visibleCheckBox;
    QCheckBox *m_desktopModeCheckBox;
    QDoubleSpinBox *m_scaleXSpinBox;
    QDoubleSpinBox *m_scaleYSpinBox;
    QDoubleSpinBox *m_rotationSpinBox;
    QDoubleSpinBox *m_shearXSpinBox;
    QDoubleSpinBox *m_shearYSpinBox;
    QCheckBox *m_followModeCheckBox;
    QComboBox *m_followWindowComboBox;
    QPushButton *m_refreshWindowsBtn;
    QPushButton *m_lockWindowBtn;
    QCheckBox *m_followBatchCheckBox;
    QComboBox *m_batchWindowComboBox;
    QPushButton *m_batchRefreshWindowsBtn;
    QPushButton *m_batchLockWindowBtn;
    QComboBox *m_followFilterTypeComboBox;
    QLineEdit *m_followFilterValueEdit;
    QPushButton *m_followFilterSuggestBtn;
    QComboBox *m_followAnchorComboBox;
    QComboBox *m_followOffsetModeComboBox;
    QDoubleSpinBox *m_followOffsetXSpinBox;
    QDoubleSpinBox *m_followOffsetYSpinBox;
    QSpinBox *m_followPollIntervalSpinBox;
    QCheckBox *m_followHideMinimizedCheckBox;

    // 事件编辑标签页
    QWidget *m_eventsTab;
    EventEditorPanel *m_eventEditor;

    // 控制按钮
    QPushButton *m_applyChangesBtn;
    QPushButton *m_cancelChangesBtn;
    QPushButton *m_loadConfigBtn;
    QPushButton *m_saveConfigBtn;

    // 数据
    QList<StickerConfig> m_configs;
    StickerConfig m_currentConfig;
    StickerConfig m_editBaseline;
    QString m_currentStickerId;
    bool m_isEditing;
    bool m_updatingEditor;
    bool m_updatingList;
    WindowRecognitionService m_windowService;
};

#endif // MAINWINDOW_H
