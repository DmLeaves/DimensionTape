#include "MainWindow.h"
#include "eventeditorpanel.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QUuid>
#include <QDebug>
#include <QSignalBlocker>
#include <QRegularExpression>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_eventEditor(nullptr)
    , m_currentStickerId("")
    , m_isEditing(false)
    , m_updatingEditor(false)
    , m_updatingList(false)
{
    setWindowTitle("桌面贴纸管理器");
    setMinimumSize(800, 600);
    resize(1300, 900);

    setupUI();
    setupMenuBar();
    setupStatusBar();

    // 连接信号
    connect(m_createStickerBtn, &QPushButton::clicked, this, &MainWindow::onCreateStickerClicked);
    connect(m_deleteStickerBtn, &QPushButton::clicked, this, &MainWindow::onDeleteStickerClicked);
    connect(m_editStickerBtn, &QPushButton::clicked, this, &MainWindow::onEditStickerClicked);
    connect(m_stickerList, &QListWidget::itemSelectionChanged, this, &MainWindow::onStickerListSelectionChanged);

    connect(m_browseImageBtn, &QPushButton::clicked, this, &MainWindow::onBrowseImageClicked);
    connect(m_browseLive2DModelBtn, &QPushButton::clicked, this, &MainWindow::onBrowseLive2DModelClicked);
    connect(m_browseLive2DRuntimeBtn, &QPushButton::clicked, this, &MainWindow::onBrowseLive2DRuntimeClicked);

    connect(m_applyChangesBtn, &QPushButton::clicked, this, &MainWindow::onApplyChangesClicked);
    connect(m_cancelChangesBtn, &QPushButton::clicked, this, &MainWindow::onCancelChangesClicked);
    connect(m_loadConfigBtn, &QPushButton::clicked, this, &MainWindow::onLoadConfigClicked);
    connect(m_saveConfigBtn, &QPushButton::clicked, this, &MainWindow::onSaveConfigClicked);

    // 初始化
    clearStickerEditor();

    qDebug() << "主窗口创建完成";
}

MainWindow::~MainWindow()
{
    qDebug() << "主窗口销毁";
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);

    m_mainSplitter = new QSplitter(Qt::Horizontal);

    setupStickerList();
    setupStickerEditor();

    m_mainSplitter->addWidget(m_stickerListWidget);
    m_mainSplitter->addWidget(m_editorWidget);
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_mainSplitter);

    // 控制按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_loadConfigBtn = new QPushButton("加载配置");
    m_saveConfigBtn = new QPushButton("保存配置");
    m_applyChangesBtn = new QPushButton("应用更改");
    m_cancelChangesBtn = new QPushButton("取消修改");

    buttonLayout->addWidget(m_loadConfigBtn);
    buttonLayout->addWidget(m_saveConfigBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelChangesBtn);
    buttonLayout->addWidget(m_applyChangesBtn);

    mainLayout->addLayout(buttonLayout);

    m_centralWidget->setLayout(mainLayout);

    connectEditorSignals();
    refreshWindowList();
    updateFollowModeUi();
}

void MainWindow::setupStickerList()
{
    m_stickerListWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_stickerListWidget);

    // 标题
    QLabel *titleLabel = new QLabel("贴纸列表");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");
    layout->addWidget(titleLabel);

    // 贴纸列表
    m_stickerList = new QListWidget;
    layout->addWidget(m_stickerList);

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_createStickerBtn = new QPushButton("创建贴纸");
    m_editStickerBtn = new QPushButton("编辑贴纸");
    m_deleteStickerBtn = new QPushButton("删除贴纸");

    m_editStickerBtn->setEnabled(false);
    m_deleteStickerBtn->setEnabled(false);

    buttonLayout->addWidget(m_createStickerBtn);
    buttonLayout->addWidget(m_editStickerBtn);
    buttonLayout->addWidget(m_deleteStickerBtn);

    layout->addLayout(buttonLayout);
}

void MainWindow::setupStickerEditor()
{
    m_editorWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_editorWidget);

    // 标题
    QLabel *titleLabel = new QLabel("贴纸编辑器");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");
    layout->addWidget(titleLabel);

    // 标签页
    m_editorTabs = new QTabWidget;

    // 基本属性标签页
    m_basicTab = new QWidget;
    setupBasicTab();
    m_editorTabs->addTab(m_basicTab, "基本属性");

    // 事件编辑标签页
    m_eventsTab = new QWidget;
    setupEventEditor();
    m_editorTabs->addTab(m_eventsTab, "事件配置");

    layout->addWidget(m_editorTabs);
}

void MainWindow::setupBasicTab()
{
    QVBoxLayout *rootLayout = new QVBoxLayout(m_basicTab);
    QScrollArea *scrollArea = new QScrollArea(m_basicTab);
    scrollArea->setWidgetResizable(true);
    QWidget *content = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(content);

    // 基本信息组
    QGroupBox *basicGroup = new QGroupBox("基本信息");
    QGridLayout *basicLayout = new QGridLayout(basicGroup);

    basicLayout->addWidget(new QLabel("名称:"), 0, 0);
    m_nameEdit = new QLineEdit;
    basicLayout->addWidget(m_nameEdit, 0, 1, 1, 2);

    basicLayout->addWidget(new QLabel("类型:"), 1, 0);
    m_contentTypeComboBox = new QComboBox;
    m_contentTypeComboBox->addItems({"图片贴纸", "Live2D贴纸"});
    basicLayout->addWidget(m_contentTypeComboBox, 1, 1, 1, 2);

    basicLayout->addWidget(new QLabel("图片路径:"), 2, 0);
    m_imagePathEdit = new QLineEdit;
    m_browseImageBtn = new QPushButton("浏览...");
    basicLayout->addWidget(m_imagePathEdit, 2, 1);
    basicLayout->addWidget(m_browseImageBtn, 2, 2);

    layout->addWidget(basicGroup);

    // Live2D 配置组
    m_live2dGroup = new QGroupBox("Live2D设置");
    QGridLayout *live2dLayout = new QGridLayout(m_live2dGroup);

    live2dLayout->addWidget(new QLabel("模型JSON:"), 0, 0);
    m_live2dModelPathEdit = new QLineEdit;
    m_browseLive2DModelBtn = new QPushButton("浏览...");
    live2dLayout->addWidget(m_live2dModelPathEdit, 0, 1);
    live2dLayout->addWidget(m_browseLive2DModelBtn, 0, 2);

    live2dLayout->addWidget(new QLabel("运行时目录:"), 1, 0);
    m_live2dRuntimeRootEdit = new QLineEdit;
    m_browseLive2DRuntimeBtn = new QPushButton("浏览...");
    live2dLayout->addWidget(m_live2dRuntimeRootEdit, 1, 1);
    live2dLayout->addWidget(m_browseLive2DRuntimeBtn, 1, 2);

    live2dLayout->addWidget(new QLabel("Shader Profile:"), 2, 0);
    m_live2dShaderProfileEdit = new QLineEdit;
    m_live2dShaderProfileEdit->setPlaceholderText("Standard");
    live2dLayout->addWidget(m_live2dShaderProfileEdit, 2, 1, 1, 2);

    layout->addWidget(m_live2dGroup);
    m_live2dGroup->setVisible(false);

    // 位置和大小组
    QGroupBox *positionGroup = new QGroupBox("位置和大小");
    QGridLayout *positionLayout = new QGridLayout(positionGroup);

    positionLayout->addWidget(new QLabel("X坐标:"), 0, 0);
    m_xSpinBox = new QSpinBox;
    m_xSpinBox->setRange(-9999, 9999);
    positionLayout->addWidget(m_xSpinBox, 0, 1);

    positionLayout->addWidget(new QLabel("Y坐标:"), 0, 2);
    m_ySpinBox = new QSpinBox;
    m_ySpinBox->setRange(-9999, 9999);
    positionLayout->addWidget(m_ySpinBox, 0, 3);

    positionLayout->addWidget(new QLabel("宽度:"), 1, 0);
    m_widthSpinBox = new QSpinBox;
    m_widthSpinBox->setRange(50, 1000);
    positionLayout->addWidget(m_widthSpinBox, 1, 1);

    positionLayout->addWidget(new QLabel("高度:"), 1, 2);
    m_heightSpinBox = new QSpinBox;
    m_heightSpinBox->setRange(50, 1000);
    positionLayout->addWidget(m_heightSpinBox, 1, 3);

    layout->addWidget(positionGroup);

    // 变换组
    QGroupBox *transformGroup = new QGroupBox("变换");
    QGridLayout *transformLayout = new QGridLayout(transformGroup);

    transformLayout->addWidget(new QLabel("缩放X:"), 0, 0);
    m_scaleXSpinBox = new QDoubleSpinBox;
    m_scaleXSpinBox->setRange(0.1, 5.0);
    m_scaleXSpinBox->setSingleStep(0.1);
    m_scaleXSpinBox->setValue(1.0);
    transformLayout->addWidget(m_scaleXSpinBox, 0, 1);

    transformLayout->addWidget(new QLabel("缩放Y:"), 0, 2);
    m_scaleYSpinBox = new QDoubleSpinBox;
    m_scaleYSpinBox->setRange(0.1, 5.0);
    m_scaleYSpinBox->setSingleStep(0.1);
    m_scaleYSpinBox->setValue(1.0);
    transformLayout->addWidget(m_scaleYSpinBox, 0, 3);

    transformLayout->addWidget(new QLabel("旋转(度):"), 1, 0);
    m_rotationSpinBox = new QDoubleSpinBox;
    m_rotationSpinBox->setRange(-360.0, 360.0);
    m_rotationSpinBox->setSingleStep(1.0);
    m_rotationSpinBox->setValue(0.0);
    transformLayout->addWidget(m_rotationSpinBox, 1, 1);

    transformLayout->addWidget(new QLabel("斜切X:"), 1, 2);
    m_shearXSpinBox = new QDoubleSpinBox;
    m_shearXSpinBox->setRange(-2.0, 2.0);
    m_shearXSpinBox->setSingleStep(0.1);
    m_shearXSpinBox->setValue(0.0);
    transformLayout->addWidget(m_shearXSpinBox, 1, 3);

    transformLayout->addWidget(new QLabel("斜切Y:"), 2, 0);
    m_shearYSpinBox = new QDoubleSpinBox;
    m_shearYSpinBox->setRange(-2.0, 2.0);
    m_shearYSpinBox->setSingleStep(0.1);
    m_shearYSpinBox->setValue(0.0);
    transformLayout->addWidget(m_shearYSpinBox, 2, 1);

    layout->addWidget(transformGroup);

    // 显示属性组
    QGroupBox *displayGroup = new QGroupBox("显示属性");
    QGridLayout *displayLayout = new QGridLayout(displayGroup);

    displayLayout->addWidget(new QLabel("透明度:"), 0, 0);
    m_opacitySpinBox = new QDoubleSpinBox;
    m_opacitySpinBox->setRange(0.1, 1.0);
    m_opacitySpinBox->setSingleStep(0.1);
    m_opacitySpinBox->setValue(1.0);
    displayLayout->addWidget(m_opacitySpinBox, 0, 1);

    m_visibleCheckBox = new QCheckBox("可见");
    m_visibleCheckBox->setChecked(true);
    displayLayout->addWidget(m_visibleCheckBox, 0, 2);

    m_desktopModeCheckBox = new QCheckBox("桌面模式");
    m_desktopModeCheckBox->setChecked(true);
    displayLayout->addWidget(m_desktopModeCheckBox, 1, 0);

    // 新增控制选项
    m_allowDragCheckBox = new QCheckBox("允许拖动");
    m_allowDragCheckBox->setChecked(true);
    displayLayout->addWidget(m_allowDragCheckBox, 1, 1);

    m_clickThroughCheckBox = new QCheckBox("点击穿透");
    m_clickThroughCheckBox->setChecked(false);
    displayLayout->addWidget(m_clickThroughCheckBox, 1, 2);

    layout->addWidget(displayGroup);

    // 跟随模式组
    QGroupBox *followGroup = new QGroupBox("跟随模式");
    QVBoxLayout *followLayout = new QVBoxLayout(followGroup);

    QGroupBox *singleFollowGroup = new QGroupBox("单个跟随");
    QGridLayout *singleLayout = new QGridLayout(singleFollowGroup);
    m_followModeCheckBox = new QCheckBox("启用单个跟随");
    singleLayout->addWidget(m_followModeCheckBox, 0, 0, 1, 2);

    singleLayout->addWidget(new QLabel("目标窗口:"), 1, 0);
    m_followWindowComboBox = new QComboBox;
    m_refreshWindowsBtn = new QPushButton("刷新");
    m_lockWindowBtn = new QPushButton("锚定窗口");
    singleLayout->addWidget(m_followWindowComboBox, 1, 1, 1, 2);
    singleLayout->addWidget(m_refreshWindowsBtn, 1, 3);
    singleLayout->addWidget(m_lockWindowBtn, 1, 4);
    followLayout->addWidget(singleFollowGroup);

    QGroupBox *batchFollowGroup = new QGroupBox("批量跟随");
    QGridLayout *batchLayout = new QGridLayout(batchFollowGroup);
    m_followBatchCheckBox = new QCheckBox("启用批量跟随");
    batchLayout->addWidget(m_followBatchCheckBox, 0, 0, 1, 2);

    batchLayout->addWidget(new QLabel("目标窗口:"), 1, 0);
    m_batchWindowComboBox = new QComboBox;
    m_batchRefreshWindowsBtn = new QPushButton("刷新");
    m_batchLockWindowBtn = new QPushButton("锚定窗口");
    batchLayout->addWidget(m_batchWindowComboBox, 1, 1, 1, 2);
    batchLayout->addWidget(m_batchRefreshWindowsBtn, 1, 3);
    batchLayout->addWidget(m_batchLockWindowBtn, 1, 4);

    batchLayout->addWidget(new QLabel("过滤类型:"), 2, 0);
    m_followFilterTypeComboBox = new QComboBox;
    m_followFilterTypeComboBox->addItems({"窗口类名", "进程名", "窗口标题(正则)"});
    batchLayout->addWidget(m_followFilterTypeComboBox, 2, 1);

    batchLayout->addWidget(new QLabel("过滤值:"), 2, 2);
    m_followFilterValueEdit = new QLineEdit;
    m_followFilterSuggestBtn = new QPushButton("生成模板");
    m_followFilterSuggestBtn->setToolTip("从目标窗口生成过滤值");
    batchLayout->addWidget(m_followFilterValueEdit, 2, 3);
    batchLayout->addWidget(m_followFilterSuggestBtn, 2, 4);
    followLayout->addWidget(batchFollowGroup);

    QGroupBox *followSettingsGroup = new QGroupBox("跟随参数");
    QGridLayout *followSettingsLayout = new QGridLayout(followSettingsGroup);

    followSettingsLayout->addWidget(new QLabel("锚点:"), 0, 0);
    m_followAnchorComboBox = new QComboBox;
    m_followAnchorComboBox->addItems({"左上", "左下", "右上", "右下"});
    followSettingsLayout->addWidget(m_followAnchorComboBox, 0, 1);

    followSettingsLayout->addWidget(new QLabel("偏移模式:"), 0, 2);
    m_followOffsetModeComboBox = new QComboBox;
    m_followOffsetModeComboBox->addItems({"像素", "比例"});
    followSettingsLayout->addWidget(m_followOffsetModeComboBox, 0, 3);

    followSettingsLayout->addWidget(new QLabel("偏移X:"), 1, 0);
    m_followOffsetXSpinBox = new QDoubleSpinBox;
    m_followOffsetXSpinBox->setRange(-99999.0, 99999.0);
    m_followOffsetXSpinBox->setDecimals(2);
    followSettingsLayout->addWidget(m_followOffsetXSpinBox, 1, 1);

    followSettingsLayout->addWidget(new QLabel("偏移Y:"), 1, 2);
    m_followOffsetYSpinBox = new QDoubleSpinBox;
    m_followOffsetYSpinBox->setRange(-99999.0, 99999.0);
    m_followOffsetYSpinBox->setDecimals(2);
    followSettingsLayout->addWidget(m_followOffsetYSpinBox, 1, 3);

    followSettingsLayout->addWidget(new QLabel("刷新间隔(ms):"), 2, 0);
    m_followPollIntervalSpinBox = new QSpinBox;
    m_followPollIntervalSpinBox->setRange(16, 5000);
    followSettingsLayout->addWidget(m_followPollIntervalSpinBox, 2, 1);

    m_followHideMinimizedCheckBox = new QCheckBox("最小化时隐藏");
    followSettingsLayout->addWidget(m_followHideMinimizedCheckBox, 2, 2, 1, 2);
    followLayout->addWidget(followSettingsGroup);

    layout->addWidget(followGroup);

    layout->addStretch();
    scrollArea->setWidget(content);
    rootLayout->addWidget(scrollArea);
}

void MainWindow::connectEditorSignals()
{
    auto intChanged = QOverload<int>::of(&QSpinBox::valueChanged);
    auto doubleChanged = QOverload<double>::of(&QDoubleSpinBox::valueChanged);

    connect(m_nameEdit, &QLineEdit::textEdited, this, &MainWindow::onEditorValueChanged);
    connect(m_contentTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onContentTypeChanged);
    connect(m_imagePathEdit, &QLineEdit::textEdited, this, &MainWindow::onEditorValueChanged);
    connect(m_live2dModelPathEdit, &QLineEdit::textEdited, this, &MainWindow::onEditorValueChanged);
    connect(m_live2dRuntimeRootEdit, &QLineEdit::textEdited, this, &MainWindow::onEditorValueChanged);
    connect(m_live2dShaderProfileEdit, &QLineEdit::textEdited, this, &MainWindow::onEditorValueChanged);
    connect(m_xSpinBox, intChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_ySpinBox, intChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_widthSpinBox, intChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_heightSpinBox, intChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_scaleXSpinBox, doubleChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_scaleYSpinBox, doubleChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_rotationSpinBox, doubleChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_shearXSpinBox, doubleChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_shearYSpinBox, doubleChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_opacitySpinBox, doubleChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_visibleCheckBox, &QCheckBox::toggled, this, &MainWindow::onEditorValueChanged);
    connect(m_desktopModeCheckBox, &QCheckBox::toggled, this, &MainWindow::onEditorValueChanged);
    connect(m_allowDragCheckBox, &QCheckBox::toggled, this, &MainWindow::onEditorValueChanged);
    connect(m_clickThroughCheckBox, &QCheckBox::toggled, this, &MainWindow::onEditorValueChanged);
    connect(m_followModeCheckBox, &QCheckBox::toggled, this, &MainWindow::onFollowModeToggled);
    connect(m_followBatchCheckBox, &QCheckBox::toggled, this, &MainWindow::onFollowBatchModeToggled);
    connect(m_followFilterTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onEditorValueChanged);
    connect(m_followFilterValueEdit, &QLineEdit::textEdited, this, &MainWindow::onEditorValueChanged);
    connect(m_followAnchorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onEditorValueChanged);
    connect(m_followOffsetModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onEditorValueChanged);
    connect(m_followOffsetXSpinBox, doubleChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_followOffsetYSpinBox, doubleChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_followPollIntervalSpinBox, intChanged, this, &MainWindow::onEditorValueChanged);
    connect(m_followHideMinimizedCheckBox, &QCheckBox::toggled, this, &MainWindow::onEditorValueChanged);
    connect(m_refreshWindowsBtn, &QPushButton::clicked, this, &MainWindow::onRefreshWindowsClicked);
    connect(m_lockWindowBtn, &QPushButton::clicked, this, &MainWindow::onLockWindowClicked);
    connect(m_batchRefreshWindowsBtn, &QPushButton::clicked,
            this, &MainWindow::onBatchRefreshWindowsClicked);
    connect(m_batchLockWindowBtn, &QPushButton::clicked,
            this, &MainWindow::onBatchLockWindowClicked);
    connect(m_followFilterSuggestBtn, &QPushButton::clicked,
            this, &MainWindow::onSuggestFollowFilterClicked);
}

void MainWindow::setupEventEditor()
{
    QVBoxLayout *layout = new QVBoxLayout(m_eventsTab);

    m_eventEditor = new EventEditorPanel(m_eventsTab);
    layout->addWidget(m_eventEditor);

    connect(m_eventEditor, &EventEditorPanel::eventsChanged,
            this, &MainWindow::onEventsChanged);
    connect(m_eventEditor, &EventEditorPanel::statusMessageRequested,
            this, &MainWindow::onEventStatusMessage);
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("文件");
    fileMenu->addAction("加载配置", this, &MainWindow::onLoadConfigClicked);
    fileMenu->addAction("保存配置", this, &MainWindow::onSaveConfigClicked);
    fileMenu->addSeparator();
    fileMenu->addAction("退出", this, &MainWindow::onExitClicked);

    QMenu *editMenu = menuBar()->addMenu("编辑");
    editMenu->addAction("创建贴纸", this, &MainWindow::onCreateStickerClicked);
    editMenu->addAction("删除贴纸", this, &MainWindow::onDeleteStickerClicked);

    QMenu *helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction("关于", [this]() {
        QMessageBox::about(this, "关于", "桌面贴纸管理器 v1.0\n\n一个简单易用的桌面贴纸工具");
    });
}

void MainWindow::onExitClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "退出程序",
        "确定要退出桌面贴纸管理器吗？\n所有贴纸将被关闭。",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        emit exitRequested();
    }
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("就绪");
}

void MainWindow::showAndRaise()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore(); // 阻止关闭，只隐藏窗口
}

void MainWindow::forceClose()
{
    // 强制关闭窗口（用于程序退出时）
    qDebug() << "主窗口强制关闭";
    QWidget::close();
}

void MainWindow::onCreateStickerClicked()
{
    StickerConfig config;
    config.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    config.name = QString("贴纸 %1").arg(m_configs.size() + 1);
    config.contentType = StickerContentType::Image;
    config.position = QPoint(100, 100);
    config.size = QSize(200, 200);
    config.isDesktopMode = true;
    config.visible = true;
    config.opacity = 1.0;

    emit createSticker(config);
}

void MainWindow::onDeleteStickerClicked()
{
    if (m_currentStickerId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要删除的贴纸");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "删除贴纸",
        "确定要删除选中的贴纸吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        emit deleteSticker(m_currentStickerId);
    }
}

void MainWindow::onEditStickerClicked()
{
    if (m_currentStickerId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要编辑的贴纸");
        return;
    }

    // 编辑功能已经通过选择列表项实现，这里可以切换到编辑标签页
    m_editorTabs->setCurrentWidget(m_basicTab);
}

void MainWindow::onStickerListSelectionChanged()
{
    if (m_updatingList) {
        return;
    }

    QList<QListWidgetItem*> selected = m_stickerList->selectedItems();
    QString previousId = m_currentStickerId;

    if (selected.isEmpty()) {
        if (m_isEditing && !previousId.isEmpty()) {
            cancelPendingEdits();
        }
        m_currentStickerId = "";
        m_editStickerBtn->setEnabled(false);
        m_deleteStickerBtn->setEnabled(false);
        clearStickerEditor();
    } else {
        QString stickerId = selected.first()->data(Qt::UserRole).toString();

        if (m_isEditing && !previousId.isEmpty() && previousId != stickerId) {
            cancelPendingEdits();
        }

        m_currentStickerId = stickerId;
        m_editStickerBtn->setEnabled(true);
        m_deleteStickerBtn->setEnabled(true);

        int index = findConfigIndex(stickerId);
        if (index >= 0) {
            m_currentConfig = m_configs.at(index);
            updateStickerEditor(m_currentConfig);
            m_editBaseline = m_currentConfig;
            m_isEditing = false;
        }
    }
}

void MainWindow::onContentTypeChanged(int index)
{
    if (m_updatingEditor) {
        return;
    }
    StickerContentType type = static_cast<StickerContentType>(index);
    updateContentTypeUi(type);
    onEditorValueChanged();
}

void MainWindow::onBrowseImageClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择贴纸图片",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        "图像文件 (*.png *.jpg *.jpeg *.bmp *.gif *.svg)"
    );

    if (!fileName.isEmpty()) {
        m_imagePathEdit->setText(fileName);
        onEditorValueChanged();
    }
}

void MainWindow::onBrowseLive2DModelClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择Live2D模型",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Live2D模型 (*.model3.json)"
    );

    if (!fileName.isEmpty()) {
        m_live2dModelPathEdit->setText(fileName);
        onEditorValueChanged();
    }
}

void MainWindow::onBrowseLive2DRuntimeClicked()
{
    QString dirName = QFileDialog::getExistingDirectory(
        this,
        "选择Live2D运行时目录",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
    );

    if (!dirName.isEmpty()) {
        m_live2dRuntimeRootEdit->setText(dirName);
        onEditorValueChanged();
    }
}

void MainWindow::onEventsChanged(const QList<StickerEvent> &events)
{
    if (m_currentStickerId.isEmpty()) {
        return;
    }
    m_currentConfig.events = events;
    applyPreviewIfEditing();
}

void MainWindow::onEventStatusMessage(const QString &message, int timeoutMs)
{
    statusBar()->showMessage(message, timeoutMs);
}

void MainWindow::beginEditSession()
{
    if (m_currentStickerId.isEmpty()) {
        return;
    }
    if (findConfigIndex(m_currentStickerId) < 0) {
        return;
    }
    if (!m_isEditing) {
        m_editBaseline = m_currentConfig;
        m_isEditing = true;
    }
}

void MainWindow::applyPreviewIfEditing()
{
    if (m_updatingEditor) {
        return;
    }
    if (m_currentStickerId.isEmpty()) {
        return;
    }
    if (findConfigIndex(m_currentStickerId) < 0) {
        return;
    }
    beginEditSession();
    StickerConfig config = getStickerConfigFromEditor();
    m_currentConfig = config;
    emit editStickerWithConfig(m_currentStickerId, config);
}

void MainWindow::cancelPendingEdits()
{
    if (!m_isEditing || m_currentStickerId.isEmpty()) {
        return;
    }
    if (findConfigIndex(m_currentStickerId) < 0) {
        m_isEditing = false;
        return;
    }
    emit editStickerWithConfig(m_currentStickerId, m_editBaseline);
    m_currentConfig = m_editBaseline;
    m_isEditing = false;
    updateStickerEditor(m_editBaseline);
}

void MainWindow::onEditorValueChanged()
{
    if (m_updatingEditor) {
        return;
    }
    if (m_contentTypeComboBox
        && m_contentTypeComboBox->currentIndex() == static_cast<int>(StickerContentType::Live2D)) {
        if (m_scaleYSpinBox && m_scaleXSpinBox
            && !qFuzzyCompare(m_scaleYSpinBox->value(), m_scaleXSpinBox->value())) {
            QSignalBlocker blocker(m_scaleYSpinBox);
            m_scaleYSpinBox->setValue(m_scaleXSpinBox->value());
        }
    }
    applyPreviewIfEditing();
}

void MainWindow::onApplyChangesClicked()
{
    if (m_currentStickerId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个贴纸");
        return;
    }

    StickerConfig config = getStickerConfigFromEditor();

    m_currentConfig = config;
    m_editBaseline = config;
    m_isEditing = false;

    // 发出编辑信号，传递完整的配置
    emit editStickerWithConfig(m_currentStickerId, config);
    statusBar()->showMessage("配置已应用", 3000);
}

void MainWindow::onCancelChangesClicked()
{
    if (m_currentStickerId.isEmpty()) {
        return;
    }
    cancelPendingEdits();
    statusBar()->showMessage("修改已取消", 2000);
}

void MainWindow::onLoadConfigClicked()
{
    emit loadStickerConfig();
}

void MainWindow::onSaveConfigClicked()
{
    emit saveStickerConfig();
}

void MainWindow::onRefreshWindowsClicked()
{
    refreshWindowList();
}

void MainWindow::onLockWindowClicked()
{
    if (m_currentStickerId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个贴纸");
        return;
    }

    if (isSingleFollowLocked()) {
        m_currentConfig.follow.targetProcessName.clear();
        updateFollowModeUi();
        emit unlockFollowTarget(m_currentStickerId);
        statusBar()->showMessage("已取消锚定", 2000);
        return;
    }

    qulonglong handle = m_followWindowComboBox->currentData().toULongLong();
    if (handle == 0) {
        QMessageBox::warning(this, "警告", "请选择要锚定的窗口");
        return;
    }

    bool followEnabled = m_followModeCheckBox->isChecked();
    if (!followEnabled) {
        QSignalBlocker blocker(m_followModeCheckBox);
        m_followModeCheckBox->setChecked(true);
        updateFollowModeUi();
        onEditorValueChanged();
    }

    WindowInfo info = m_windowService.queryWindow(static_cast<WindowHandle>(handle));
    if (!info.processName.isEmpty()) {
        m_currentConfig.follow.targetProcessName = info.processName;
        updateFollowModeUi();
    }

    emit lockFollowTarget(m_currentStickerId, handle);
    statusBar()->showMessage("目标窗口已锚定", 2000);
}

void MainWindow::onBatchRefreshWindowsClicked()
{
    refreshWindowList();
}

void MainWindow::onBatchLockWindowClicked()
{
    if (m_currentStickerId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个贴纸");
        return;
    }

    if (isBatchFollowLocked()) {
        m_currentConfig.follow.targetProcessName.clear();
        updateFollowModeUi();
        emit unlockFollowTarget(m_currentStickerId);
        statusBar()->showMessage("已取消锚定", 2000);
        return;
    }

    qulonglong handle = m_batchWindowComboBox->currentData().toULongLong();
    if (handle == 0) {
        QMessageBox::warning(this, "警告", "请选择要锚定的窗口");
        return;
    }

    bool batchEnabled = m_followBatchCheckBox->isChecked();
    if (!batchEnabled) {
        QSignalBlocker blocker(m_followBatchCheckBox);
        m_followBatchCheckBox->setChecked(true);
        updateFollowModeUi();
        onEditorValueChanged();
    }

    if (m_followFilterValueEdit->text().trimmed().isEmpty()) {
        if (!applyFollowFilterSuggestion(handle, true, true)) {
            return;
        }
    }

    WindowInfo info = m_windowService.queryWindow(static_cast<WindowHandle>(handle));
    if (!info.processName.isEmpty()) {
        m_currentConfig.follow.targetProcessName = info.processName;
        updateFollowModeUi();
    }

    emit lockFollowTarget(m_currentStickerId, handle);
    statusBar()->showMessage("目标窗口已锚定", 2000);
}

void MainWindow::onSuggestFollowFilterClicked()
{
    if (m_batchWindowComboBox->count() == 0) {
        refreshWindowList();
    }

    qulonglong handle = m_batchWindowComboBox->currentData().toULongLong();
    if (!applyFollowFilterSuggestion(handle, true, true)) {
        return;
    }
    statusBar()->showMessage("过滤值模板已生成", 2000);
}

void MainWindow::onFollowModeToggled(bool enabled)
{
    if (m_updatingEditor) {
        return;
    }

    if (enabled) {
        QSignalBlocker blocker(m_followBatchCheckBox);
        m_followBatchCheckBox->setChecked(false);
    }

    if (enabled) {
        if (m_followWindowComboBox->count() == 0) {
            refreshWindowList();
        }
        if (!m_currentConfig.follow.enabled && m_followPollIntervalSpinBox->value() == 100) {
            QSignalBlocker blocker(m_followPollIntervalSpinBox);
            m_followPollIntervalSpinBox->setValue(16);
        }
        QSignalBlocker blocker(m_desktopModeCheckBox);
        m_desktopModeCheckBox->setChecked(false);
    }

    updateFollowModeUi();
    onEditorValueChanged();
}

void MainWindow::onFollowBatchModeToggled(bool)
{
    if (m_updatingEditor) {
        return;
    }
    bool isLive2D = m_contentTypeComboBox
        && m_contentTypeComboBox->currentIndex() == static_cast<int>(StickerContentType::Live2D);
    if (isLive2D) {
        QSignalBlocker blocker(m_followBatchCheckBox);
        m_followBatchCheckBox->setChecked(false);
        updateFollowModeUi();
        statusBar()->showMessage("Live2D不支持批量跟随", 2000);
        return;
    }
    bool enabled = m_followBatchCheckBox->isChecked();
    if (enabled) {
        QSignalBlocker blocker(m_followModeCheckBox);
        m_followModeCheckBox->setChecked(false);
        if (m_batchWindowComboBox->count() == 0) {
            refreshWindowList();
        }
        if (!m_currentConfig.follow.enabled && m_followPollIntervalSpinBox->value() == 100) {
            QSignalBlocker blockerInterval(m_followPollIntervalSpinBox);
            m_followPollIntervalSpinBox->setValue(16);
        }
        QSignalBlocker blockerDesktop(m_desktopModeCheckBox);
        m_desktopModeCheckBox->setChecked(false);
    }
    updateFollowModeUi();
    onEditorValueChanged();
}

void MainWindow::onStickerCreated(const StickerConfig &config)
{
    int index = findConfigIndex(config.id);
    if (index >= 0) {
        m_configs[index] = config;
    } else {
        m_configs.append(config);
    }

    m_currentStickerId = config.id;
    m_currentConfig = config;
    m_editBaseline = config;
    m_isEditing = false;

    updateStickerList();
    updateStickerEditor(config);
    statusBar()->showMessage(QString("贴纸 '%1' 已创建").arg(config.name), 3000);
}

void MainWindow::onStickerDeleted(const QString &stickerId)
{
    int index = findConfigIndex(stickerId);
    if (index >= 0) {
        m_configs.removeAt(index);
    }

    if (m_currentStickerId == stickerId) {
        m_isEditing = false;
        if (!m_configs.isEmpty()) {
            m_currentConfig = m_configs.first();
            m_currentStickerId = m_currentConfig.id;
            m_editBaseline = m_currentConfig;
            updateStickerEditor(m_currentConfig);
        } else {
            m_currentStickerId = "";
            m_currentConfig = StickerConfig();
            m_editBaseline = StickerConfig();
            clearStickerEditor();
        }
    }

    updateStickerList();
    statusBar()->showMessage("贴纸已删除", 3000);
}

void MainWindow::onStickerConfigChanged(const StickerConfig &config)
{
    int index = findConfigIndex(config.id);
    if (index >= 0) {
        m_configs[index] = config;
    } else {
        m_configs.append(config);
    }

    if (m_currentStickerId.isEmpty()) {
        m_currentStickerId = config.id;
        m_currentConfig = config;
        m_editBaseline = config;
        m_isEditing = false;
    }

    updateStickerList();

    // 如果是当前选中的贴纸，更新编辑器
    if (m_currentStickerId == config.id) {
        m_currentConfig = config;
        updateStickerEditor(config);
        if (!m_isEditing) {
            m_editBaseline = config;
        }
    }
}

void MainWindow::updateStickerEditor(const StickerConfig &config)
{
    m_updatingEditor = true;
    m_nameEdit->setText(config.name);
    m_contentTypeComboBox->setCurrentIndex(static_cast<int>(config.contentType));
    m_imagePathEdit->setText(config.imagePath);
    m_live2dModelPathEdit->setText(config.live2d.modelJsonPath);
    m_live2dRuntimeRootEdit->setText(config.live2d.runtimeRoot);
    m_live2dShaderProfileEdit->setText(
        config.live2d.shaderProfile.isEmpty() ? "Standard" : config.live2d.shaderProfile);
    m_xSpinBox->setValue(config.position.x());
    m_ySpinBox->setValue(config.position.y());
    m_widthSpinBox->setValue(config.size.width());
    m_heightSpinBox->setValue(config.size.height());
    m_scaleXSpinBox->setValue(config.transform.scaleX);
    m_scaleYSpinBox->setValue(config.transform.scaleY);
    m_rotationSpinBox->setValue(config.transform.rotation);
    m_shearXSpinBox->setValue(config.transform.shearX);
    m_shearYSpinBox->setValue(config.transform.shearY);
    m_opacitySpinBox->setValue(config.opacity);
    m_visibleCheckBox->setChecked(config.visible);
    m_desktopModeCheckBox->setChecked(config.isDesktopMode);
    m_allowDragCheckBox->setChecked(config.allowDrag);        // 新增
    m_clickThroughCheckBox->setChecked(config.clickThrough);  // 新增
    m_followModeCheckBox->setChecked(config.follow.enabled && !config.follow.batchMode);
    m_followBatchCheckBox->setChecked(config.follow.enabled && config.follow.batchMode);
    m_followFilterTypeComboBox->setCurrentIndex(static_cast<int>(config.follow.filterType));
    m_followFilterValueEdit->setText(config.follow.filterValue);
    m_followAnchorComboBox->setCurrentIndex(static_cast<int>(config.follow.anchor));
    m_followOffsetModeComboBox->setCurrentIndex(static_cast<int>(config.follow.offsetMode));
    m_followOffsetXSpinBox->setValue(config.follow.offset.x());
    m_followOffsetYSpinBox->setValue(config.follow.offset.y());
    m_followPollIntervalSpinBox->setValue(config.follow.pollIntervalMs);
    m_followHideMinimizedCheckBox->setChecked(config.follow.hideWhenMinimized);
    updateFollowModeUi();
    updateContentTypeUi(config.contentType);

    if (m_eventEditor) {
        m_eventEditor->setEvents(config.events);
        m_eventEditor->setEditingEnabled(true);
    }
    m_updatingEditor = false;
}

void MainWindow::clearStickerEditor()
{
    m_updatingEditor = true;
    m_nameEdit->clear();
    m_contentTypeComboBox->setCurrentIndex(static_cast<int>(StickerContentType::Image));
    m_imagePathEdit->clear();
    m_live2dModelPathEdit->clear();
    m_live2dRuntimeRootEdit->clear();
    m_live2dShaderProfileEdit->setText("Standard");
    m_xSpinBox->setValue(0);
    m_ySpinBox->setValue(0);
    m_widthSpinBox->setValue(200);
    m_heightSpinBox->setValue(200);
    m_scaleXSpinBox->setValue(1.0);
    m_scaleYSpinBox->setValue(1.0);
    m_rotationSpinBox->setValue(0.0);
    m_shearXSpinBox->setValue(0.0);
    m_shearYSpinBox->setValue(0.0);
    m_opacitySpinBox->setValue(1.0);
    m_visibleCheckBox->setChecked(true);
    m_desktopModeCheckBox->setChecked(true);
    m_allowDragCheckBox->setChecked(true);      // 新增
    m_clickThroughCheckBox->setChecked(false);  // 新增
    m_followModeCheckBox->setChecked(false);
    m_followBatchCheckBox->setChecked(false);
    m_followFilterTypeComboBox->setCurrentIndex(static_cast<int>(FollowFilterType::WindowClass));
    m_followFilterValueEdit->clear();
    m_followAnchorComboBox->setCurrentIndex(static_cast<int>(FollowAnchor::LeftTop));
    m_followOffsetModeComboBox->setCurrentIndex(static_cast<int>(FollowOffsetMode::AbsolutePixels));
    m_followOffsetXSpinBox->setValue(0.0);
    m_followOffsetYSpinBox->setValue(0.0);
    m_followPollIntervalSpinBox->setValue(16);
    m_followHideMinimizedCheckBox->setChecked(true);
    updateFollowModeUi();
    updateContentTypeUi(StickerContentType::Image);

    if (m_eventEditor) {
        m_eventEditor->setEvents(QList<StickerEvent>());
        m_eventEditor->clearInputs();
        m_eventEditor->setEditingEnabled(false);
    }
    m_updatingEditor = false;
}

StickerConfig MainWindow::getStickerConfigFromEditor() const
{
    StickerConfig config;

    config = m_currentConfig;

    // 更新编辑器中的值
    config.name = m_nameEdit->text();
    config.contentType = static_cast<StickerContentType>(m_contentTypeComboBox->currentIndex());
    config.imagePath = m_imagePathEdit->text();
    config.live2d.modelJsonPath = m_live2dModelPathEdit->text().trimmed();
    config.live2d.runtimeRoot = m_live2dRuntimeRootEdit->text().trimmed();
    config.live2d.shaderProfile = m_live2dShaderProfileEdit->text().trimmed();
    if (config.live2d.shaderProfile.isEmpty()) {
        config.live2d.shaderProfile = "Standard";
    }
    config.position = QPoint(m_xSpinBox->value(), m_ySpinBox->value());
    config.size = QSize(m_widthSpinBox->value(), m_heightSpinBox->value());
    config.transform.scaleX = m_scaleXSpinBox->value();
    config.transform.scaleY = m_scaleYSpinBox->value();
    config.transform.rotation = m_rotationSpinBox->value();
    config.transform.shearX = m_shearXSpinBox->value();
    config.transform.shearY = m_shearYSpinBox->value();
    config.opacity = m_opacitySpinBox->value();
    config.visible = m_visibleCheckBox->isChecked();
    config.isDesktopMode = m_desktopModeCheckBox->isChecked();
    config.allowDrag = m_allowDragCheckBox->isChecked();        // 新增
    config.clickThrough = m_clickThroughCheckBox->isChecked();  // 新增
    bool singleEnabled = m_followModeCheckBox->isChecked();
    bool batchEnabled = m_followBatchCheckBox->isChecked();
    if (config.contentType == StickerContentType::Live2D && batchEnabled) {
        batchEnabled = false;
    }
    config.follow.enabled = singleEnabled || batchEnabled;
    config.follow.batchMode = batchEnabled;
    config.follow.filterType = static_cast<FollowFilterType>(m_followFilterTypeComboBox->currentIndex());
    config.follow.filterValue = m_followFilterValueEdit->text().trimmed();
    config.follow.targetProcessName = m_currentConfig.follow.targetProcessName;
    config.follow.anchor = static_cast<FollowAnchor>(m_followAnchorComboBox->currentIndex());
    config.follow.offsetMode = static_cast<FollowOffsetMode>(m_followOffsetModeComboBox->currentIndex());
    config.follow.offset = QPointF(m_followOffsetXSpinBox->value(), m_followOffsetYSpinBox->value());
    config.follow.pollIntervalMs = m_followPollIntervalSpinBox->value();
    config.follow.hideWhenMinimized = m_followHideMinimizedCheckBox->isChecked();

    if (config.follow.enabled) {
        config.isDesktopMode = false;
    }

    if (config.contentType == StickerContentType::Live2D) {
        config.clickThrough = false;
        if (!qFuzzyCompare(config.transform.scaleY, config.transform.scaleX)) {
            config.transform.scaleY = config.transform.scaleX;
        }
        if (config.live2d.baseSize.isEmpty()) {
            config.live2d.baseSize = config.size;
        }
    }

    return config;
}

void MainWindow::refreshWindowList()
{
    populateWindowCombo(m_followWindowComboBox);
    populateWindowCombo(m_batchWindowComboBox);
}

void MainWindow::populateWindowCombo(QComboBox *combo)
{
    if (!combo) {
        return;
    }

    qulonglong currentHandle = combo->currentData().toULongLong();
    combo->clear();

    QList<WindowInfo> windows = m_windowService.listWindows(true);
    if (windows.isEmpty()) {
        combo->addItem("没有可用窗口", QVariant::fromValue<qulonglong>(0));
        return;
    }

    int selectedIndex = -1;
    for (const WindowInfo &info : windows) {
        QString title = info.title.trimmed();
        if (title.isEmpty()) {
            title = info.processName;
        }
        QString label = QString("%1 (%2)").arg(title, info.processName);
        combo->addItem(label, QVariant::fromValue<qulonglong>(info.handle));
        if (currentHandle != 0 && info.handle == currentHandle) {
            selectedIndex = combo->count() - 1;
        }
    }

    if (selectedIndex >= 0) {
        combo->setCurrentIndex(selectedIndex);
    }
}

void MainWindow::updateFollowModeUi()
{
    bool singleEnabled = m_followModeCheckBox->isChecked();
    bool batchEnabled = m_followBatchCheckBox->isChecked();
    bool isLive2D = m_contentTypeComboBox
        && m_contentTypeComboBox->currentIndex() == static_cast<int>(StickerContentType::Live2D);
    if (isLive2D && batchEnabled) {
        QSignalBlocker blocker(m_followBatchCheckBox);
        m_followBatchCheckBox->setChecked(false);
        batchEnabled = false;
    }
    bool followEnabled = singleEnabled || batchEnabled;
    bool singleLocked = isSingleFollowLocked();
    bool batchLocked = isBatchFollowLocked();

    if (followEnabled) {
        QSignalBlocker blocker(m_desktopModeCheckBox);
        m_desktopModeCheckBox->setChecked(false);
    }

    m_desktopModeCheckBox->setEnabled(!followEnabled);
    m_followModeCheckBox->setEnabled(!batchEnabled || singleEnabled);
    m_followBatchCheckBox->setEnabled(!isLive2D && (!singleEnabled || batchEnabled));
    m_followWindowComboBox->setEnabled(singleEnabled);
    m_refreshWindowsBtn->setEnabled(singleEnabled);
    m_lockWindowBtn->setEnabled(singleEnabled);
    if (m_lockWindowBtn) {
        m_lockWindowBtn->setText(singleLocked ? "取消锚定" : "锚定窗口");
    }
    m_batchWindowComboBox->setEnabled(batchEnabled && !isLive2D);
    m_batchRefreshWindowsBtn->setEnabled(batchEnabled && !isLive2D);
    m_batchLockWindowBtn->setEnabled(batchEnabled && !isLive2D);
    if (m_batchLockWindowBtn) {
        m_batchLockWindowBtn->setText(batchLocked ? "取消锚定" : "锚定窗口");
    }
    m_followFilterTypeComboBox->setEnabled(batchEnabled && !isLive2D);
    m_followFilterValueEdit->setEnabled(batchEnabled && !isLive2D);
    m_followFilterSuggestBtn->setEnabled(batchEnabled && !isLive2D);
    m_followAnchorComboBox->setEnabled(followEnabled);
    m_followOffsetModeComboBox->setEnabled(followEnabled);
    m_followOffsetXSpinBox->setEnabled(followEnabled);
    m_followOffsetYSpinBox->setEnabled(followEnabled);
    m_followPollIntervalSpinBox->setEnabled(followEnabled);
    m_followHideMinimizedCheckBox->setEnabled(followEnabled);
}

void MainWindow::updateContentTypeUi(StickerContentType type)
{
    bool isLive2D = (type == StickerContentType::Live2D);
    if (m_imagePathEdit) {
        m_imagePathEdit->setEnabled(!isLive2D);
    }
    if (m_browseImageBtn) {
        m_browseImageBtn->setEnabled(!isLive2D);
    }
    if (m_live2dGroup) {
        m_live2dGroup->setVisible(isLive2D);
        m_live2dGroup->setEnabled(isLive2D);
    }
    if (isLive2D && m_live2dShaderProfileEdit
        && m_live2dShaderProfileEdit->text().trimmed().isEmpty()) {
        QSignalBlocker blocker(m_live2dShaderProfileEdit);
        m_live2dShaderProfileEdit->setText("Standard");
    }
    if (m_clickThroughCheckBox) {
        QSignalBlocker blocker(m_clickThroughCheckBox);
        if (isLive2D) {
            m_clickThroughCheckBox->setChecked(false);
        }
        m_clickThroughCheckBox->setEnabled(!isLive2D);
    }
    if (m_scaleYSpinBox) {
        QSignalBlocker blocker(m_scaleYSpinBox);
        m_scaleYSpinBox->setEnabled(!isLive2D);
        if (isLive2D && m_scaleXSpinBox) {
            m_scaleYSpinBox->setValue(m_scaleXSpinBox->value());
        }
    }
    updateFollowModeUi();
}

bool MainWindow::isSingleFollowLocked() const
{
    if (!m_followModeCheckBox) {
        return false;
    }
    if (!m_followModeCheckBox->isChecked()) {
        return false;
    }
    return !m_currentConfig.follow.targetProcessName.trimmed().isEmpty();
}

bool MainWindow::isBatchFollowLocked() const
{
    if (!m_followBatchCheckBox) {
        return false;
    }
    if (!m_followBatchCheckBox->isChecked()) {
        return false;
    }
    return !m_currentConfig.follow.targetProcessName.trimmed().isEmpty();
}

QString MainWindow::buildFollowFilterValue(const WindowInfo &info) const
{
    int typeIndex = m_followFilterTypeComboBox->currentIndex();
    if (typeIndex == static_cast<int>(FollowFilterType::WindowClass)) {
        return info.className.trimmed();
    }
    if (typeIndex == static_cast<int>(FollowFilterType::ProcessName)) {
        return info.processName.trimmed();
    }
    return QRegularExpression::escape(info.title.trimmed());
}

bool MainWindow::applyFollowFilterSuggestion(qulonglong handle, bool force, bool warnOnEmpty)
{
    if (handle == 0) {
        if (warnOnEmpty) {
            QMessageBox::warning(this, "警告", "请选择要参考的窗口");
        }
        return false;
    }

    if (!force && !m_followFilterValueEdit->text().trimmed().isEmpty()) {
        return false;
    }

    WindowInfo info = m_windowService.queryWindow(static_cast<WindowHandle>(handle));
    QString value = buildFollowFilterValue(info);
    if (value.isEmpty()) {
        if (warnOnEmpty) {
            QMessageBox::warning(this, "警告", "无法从目标窗口提取过滤条件");
        }
        return false;
    }

    {
        QSignalBlocker blocker(m_followFilterValueEdit);
        m_followFilterValueEdit->setText(value);
    }
    applyPreviewIfEditing();
    return true;
}

void MainWindow::updateStickerList()
{
    QString currentId = m_currentStickerId; // 保存当前选择
    m_updatingList = true;

    m_stickerList->clear();
    for (const StickerConfig &config : m_configs) {
        QListWidgetItem *item = new QListWidgetItem(config.name);
        item->setData(Qt::UserRole, config.id);
        if (!config.visible) {
            item->setTextColor(QColor(128, 128, 128));
        }
        m_stickerList->addItem(item);
        if (config.id == currentId) {
            item->setSelected(true);
        }
    }

    m_updatingList = false;
    statusBar()->showMessage(QString("共 %1 个贴纸").arg(m_configs.size()), 3000);
}

void MainWindow::onStickerConfigsUpdated(const QList<StickerConfig> &configs)
{
    qDebug() << "更新贴纸列表，共" << configs.size() << "个贴纸";

    m_configs = configs;
    if (m_configs.isEmpty()) {
        m_currentStickerId = "";
        m_currentConfig = StickerConfig();
        m_editBaseline = StickerConfig();
        m_isEditing = false;
        clearStickerEditor();
        updateStickerList();
        return;
    }

    int index = findConfigIndex(m_currentStickerId);
    if (index < 0) {
        m_currentConfig = m_configs.first();
        m_currentStickerId = m_currentConfig.id;
        m_isEditing = false;
        m_editBaseline = m_currentConfig;
        updateStickerEditor(m_currentConfig);
    } else {
        m_currentConfig = m_configs.at(index);
        if (!m_isEditing) {
            m_editBaseline = m_currentConfig;
        }
        updateStickerEditor(m_currentConfig);
    }

    updateStickerList();
}

int MainWindow::findConfigIndex(const QString &stickerId) const
{
    for (int i = 0; i < m_configs.size(); ++i) {
        if (m_configs.at(i).id == stickerId) {
            return i;
        }
    }
    return -1;
}
