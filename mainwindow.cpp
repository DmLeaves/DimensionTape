#include "MainWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QUuid>
#include <QDebug>
#include <QSignalBlocker>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_currentStickerId("")
    , m_isEditing(false)
    , m_updatingEditor(false)
    , m_updatingList(false)
{
    setWindowTitle("桌面贴纸管理器");
    setMinimumSize(800, 600);
    resize(1200, 750);

    setupUI();
    setupMenuBar();
    setupStatusBar();

    // 连接信号
    connect(m_createStickerBtn, &QPushButton::clicked, this, &MainWindow::onCreateStickerClicked);
    connect(m_deleteStickerBtn, &QPushButton::clicked, this, &MainWindow::onDeleteStickerClicked);
    connect(m_editStickerBtn, &QPushButton::clicked, this, &MainWindow::onEditStickerClicked);
    connect(m_stickerList, &QListWidget::itemSelectionChanged, this, &MainWindow::onStickerListSelectionChanged);

    connect(m_browseImageBtn, &QPushButton::clicked, this, &MainWindow::onBrowseImageClicked);
    connect(m_addEventBtn, &QPushButton::clicked, this, &MainWindow::onAddEventClicked);
    connect(m_removeEventBtn, &QPushButton::clicked, this, &MainWindow::onRemoveEventClicked);
    connect(m_eventTable, &QTableWidget::itemChanged, this, &MainWindow::onEventTableItemChanged);

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
    QVBoxLayout *layout = new QVBoxLayout(m_basicTab);

    // 基本信息组
    QGroupBox *basicGroup = new QGroupBox("基本信息");
    QGridLayout *basicLayout = new QGridLayout(basicGroup);

    basicLayout->addWidget(new QLabel("名称:"), 0, 0);
    m_nameEdit = new QLineEdit;
    basicLayout->addWidget(m_nameEdit, 0, 1, 1, 2);

    basicLayout->addWidget(new QLabel("图片路径:"), 1, 0);
    m_imagePathEdit = new QLineEdit;
    m_browseImageBtn = new QPushButton("浏览...");
    basicLayout->addWidget(m_imagePathEdit, 1, 1);
    basicLayout->addWidget(m_browseImageBtn, 1, 2);

    layout->addWidget(basicGroup);

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
    QGridLayout *followLayout = new QGridLayout(followGroup);

    m_followModeCheckBox = new QCheckBox("启用跟随模式");
    followLayout->addWidget(m_followModeCheckBox, 0, 0, 1, 2);

    followLayout->addWidget(new QLabel("目标窗口:"), 1, 0);
    m_followWindowComboBox = new QComboBox;
    m_refreshWindowsBtn = new QPushButton("刷新");
    m_lockWindowBtn = new QPushButton("锚定窗口");
    followLayout->addWidget(m_followWindowComboBox, 1, 1, 1, 2);
    followLayout->addWidget(m_refreshWindowsBtn, 1, 3);
    followLayout->addWidget(m_lockWindowBtn, 1, 4);

    m_followBatchCheckBox = new QCheckBox("批量跟随");
    followLayout->addWidget(m_followBatchCheckBox, 2, 0);

    followLayout->addWidget(new QLabel("过滤类型:"), 3, 0);
    m_followFilterTypeComboBox = new QComboBox;
    m_followFilterTypeComboBox->addItems({"窗口类名", "进程名", "窗口标题(正则)"});
    followLayout->addWidget(m_followFilterTypeComboBox, 3, 1);

    followLayout->addWidget(new QLabel("过滤值:"), 3, 2);
    m_followFilterValueEdit = new QLineEdit;
    followLayout->addWidget(m_followFilterValueEdit, 3, 3, 1, 2);

    followLayout->addWidget(new QLabel("锚点:"), 4, 0);
    m_followAnchorComboBox = new QComboBox;
    m_followAnchorComboBox->addItems({"左上", "左下", "右上", "右下"});
    followLayout->addWidget(m_followAnchorComboBox, 4, 1);

    followLayout->addWidget(new QLabel("偏移模式:"), 4, 2);
    m_followOffsetModeComboBox = new QComboBox;
    m_followOffsetModeComboBox->addItems({"像素", "比例"});
    followLayout->addWidget(m_followOffsetModeComboBox, 4, 3);

    followLayout->addWidget(new QLabel("偏移X:"), 5, 0);
    m_followOffsetXSpinBox = new QDoubleSpinBox;
    m_followOffsetXSpinBox->setRange(-99999.0, 99999.0);
    m_followOffsetXSpinBox->setDecimals(2);
    followLayout->addWidget(m_followOffsetXSpinBox, 5, 1);

    followLayout->addWidget(new QLabel("偏移Y:"), 5, 2);
    m_followOffsetYSpinBox = new QDoubleSpinBox;
    m_followOffsetYSpinBox->setRange(-99999.0, 99999.0);
    m_followOffsetYSpinBox->setDecimals(2);
    followLayout->addWidget(m_followOffsetYSpinBox, 5, 3);

    followLayout->addWidget(new QLabel("刷新间隔(ms):"), 6, 0);
    m_followPollIntervalSpinBox = new QSpinBox;
    m_followPollIntervalSpinBox->setRange(16, 5000);
    followLayout->addWidget(m_followPollIntervalSpinBox, 6, 1);

    m_followHideMinimizedCheckBox = new QCheckBox("最小化时隐藏");
    followLayout->addWidget(m_followHideMinimizedCheckBox, 6, 2, 1, 2);

    layout->addWidget(followGroup);

    layout->addStretch();
}

void MainWindow::connectEditorSignals()
{
    auto intChanged = QOverload<int>::of(&QSpinBox::valueChanged);
    auto doubleChanged = QOverload<double>::of(&QDoubleSpinBox::valueChanged);

    connect(m_nameEdit, &QLineEdit::textEdited, this, &MainWindow::onEditorValueChanged);
    connect(m_imagePathEdit, &QLineEdit::textEdited, this, &MainWindow::onEditorValueChanged);
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
}

void MainWindow::setupEventEditor()
{
    QVBoxLayout *layout = new QVBoxLayout(m_eventsTab);

    // 事件表格
    m_eventTable = new QTableWidget;
    m_eventTable->setColumnCount(5);
    QStringList headers = {"触发器", "事件类型", "目标", "参数", "启用"};
    m_eventTable->setHorizontalHeaderLabels(headers);
    m_eventTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_eventTable);

    // 添加事件控制
    QGroupBox *addEventGroup = new QGroupBox("添加新事件");
    QGridLayout *addLayout = new QGridLayout(addEventGroup);

    addLayout->addWidget(new QLabel("触发器:"), 0, 0);
    m_triggerComboBox = new QComboBox;
    m_triggerComboBox->addItems({"左键单击", "右键单击", "双击", "滚轮向上", "滚轮向下", "鼠标进入", "鼠标离开"});
    addLayout->addWidget(m_triggerComboBox, 0, 1);

    addLayout->addWidget(new QLabel("事件类型:"), 0, 2);
    m_eventTypeComboBox = new QComboBox;
    m_eventTypeComboBox->addItems({"打开程序", "打开文件夹", "打开文件", "播放声音", "显示消息", "自定义脚本"});
    addLayout->addWidget(m_eventTypeComboBox, 0, 3);

    addLayout->addWidget(new QLabel("目标:"), 1, 0);
    m_targetEdit = new QLineEdit;
    addLayout->addWidget(m_targetEdit, 1, 1, 1, 2);

    addLayout->addWidget(new QLabel("参数:"), 1, 3);
    m_parametersEdit = new QLineEdit;
    addLayout->addWidget(m_parametersEdit, 1, 4);

    layout->addWidget(addEventGroup);

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_addEventBtn = new QPushButton("添加事件");
    m_removeEventBtn = new QPushButton("删除事件");

    buttonLayout->addWidget(m_addEventBtn);
    buttonLayout->addWidget(m_removeEventBtn);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);
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
    }
}

void MainWindow::onAddEventClicked()
{
    if (m_currentStickerId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个贴纸");
        return;
    }

    if (m_targetEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入事件目标");
        return;
    }

    StickerEvent event;
    event.trigger = stringToMouseTrigger(m_triggerComboBox->currentText());
    event.type = stringToEventType(m_eventTypeComboBox->currentText());
    event.target = m_targetEdit->text();
    event.parameters = m_parametersEdit->text();
    event.enabled = true;

    m_currentConfig.events.append(event);
    updateEventTable();
    applyPreviewIfEditing();

    // 清空输入框
    m_targetEdit->clear();
    m_parametersEdit->clear();

    statusBar()->showMessage("事件已添加", 2000);
}

void MainWindow::onRemoveEventClicked()
{
    int currentRow = m_eventTable->currentRow();
    if (currentRow >= 0 && !m_currentStickerId.isEmpty()) {
        if (currentRow < m_currentConfig.events.size()) {
            m_currentConfig.events.removeAt(currentRow);
            updateEventTable();
            statusBar()->showMessage("事件已删除", 2000);
            applyPreviewIfEditing();
        }
    } else {
        QMessageBox::warning(this, "警告", "请选择要删除的事件");
    }
}

void MainWindow::onEventTableItemChanged(QTableWidgetItem *item)
{
    if (!item || m_currentStickerId.isEmpty()) {
        return;
    }

    int row = item->row();
    int column = item->column();

    // 找到对应的配置
    if (row < m_currentConfig.events.size()) {
        StickerEvent &event = m_currentConfig.events[row];

        switch (column) {
        case 0:
            event.trigger = stringToMouseTrigger(item->text());
            break;
        case 1:
            event.type = stringToEventType(item->text());
            break;
        case 2:
            event.target = item->text();
            break;
        case 3:
            event.parameters = item->text();
            break;
        case 4:
            event.enabled = (item->checkState() == Qt::Checked);
            break;
        }

        statusBar()->showMessage("事件已更新", 2000);
        applyPreviewIfEditing();
    }
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

    qulonglong handle = m_followWindowComboBox->currentData().toULongLong();
    if (handle == 0) {
        QMessageBox::warning(this, "警告", "请选择要锚定的窗口");
        return;
    }

    bool followEnabled = m_followModeCheckBox->isChecked();
    bool batchEnabled = m_followBatchCheckBox->isChecked();
    QString filterValue = m_followFilterValueEdit->text().trimmed();

    if (batchEnabled && filterValue.isEmpty()) {
        WindowInfo info = m_windowService.queryWindow(static_cast<WindowHandle>(handle));
        QString value;
        int typeIndex = m_followFilterTypeComboBox->currentIndex();
        if (typeIndex == static_cast<int>(FollowFilterType::WindowClass)) {
            value = info.className;
        } else if (typeIndex == static_cast<int>(FollowFilterType::ProcessName)) {
            value = info.processName;
        } else {
            value = QRegularExpression::escape(info.title);
        }

        if (value.isEmpty()) {
            QMessageBox::warning(this, "警告", "无法从目标窗口提取过滤条件");
            return;
        }

        {
            QSignalBlocker blocker(m_followFilterValueEdit);
            m_followFilterValueEdit->setText(value);
        }
        applyPreviewIfEditing();
    }

    if (!followEnabled) {
        QSignalBlocker blocker(m_followModeCheckBox);
        m_followModeCheckBox->setChecked(true);
        updateFollowModeUi();
        onEditorValueChanged();
    }

    if (batchEnabled && m_followFilterValueEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "批量跟随需要填写过滤值");
        return;
    }

    emit lockFollowTarget(m_currentStickerId, handle);
    statusBar()->showMessage("目标窗口已锚定", 2000);
}

void MainWindow::onFollowModeToggled(bool enabled)
{
    if (m_updatingEditor) {
        return;
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
    m_imagePathEdit->setText(config.imagePath);
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
    m_followModeCheckBox->setChecked(config.follow.enabled);
    m_followBatchCheckBox->setChecked(config.follow.batchMode);
    m_followFilterTypeComboBox->setCurrentIndex(static_cast<int>(config.follow.filterType));
    m_followFilterValueEdit->setText(config.follow.filterValue);
    m_followAnchorComboBox->setCurrentIndex(static_cast<int>(config.follow.anchor));
    m_followOffsetModeComboBox->setCurrentIndex(static_cast<int>(config.follow.offsetMode));
    m_followOffsetXSpinBox->setValue(config.follow.offset.x());
    m_followOffsetYSpinBox->setValue(config.follow.offset.y());
    m_followPollIntervalSpinBox->setValue(config.follow.pollIntervalMs);
    m_followHideMinimizedCheckBox->setChecked(config.follow.hideWhenMinimized);
    updateFollowModeUi();

    updateEventTable();
    m_updatingEditor = false;
}

void MainWindow::clearStickerEditor()
{
    m_updatingEditor = true;
    m_nameEdit->clear();
    m_imagePathEdit->clear();
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

    m_eventTable->setRowCount(0);
    m_updatingEditor = false;
}

StickerConfig MainWindow::getStickerConfigFromEditor() const
{
    StickerConfig config;

    config = m_currentConfig;

    // 更新编辑器中的值
    config.name = m_nameEdit->text();
    config.imagePath = m_imagePathEdit->text();
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
    config.follow.enabled = m_followModeCheckBox->isChecked();
    config.follow.batchMode = m_followBatchCheckBox->isChecked();
    config.follow.filterType = static_cast<FollowFilterType>(m_followFilterTypeComboBox->currentIndex());
    config.follow.filterValue = m_followFilterValueEdit->text().trimmed();
    config.follow.anchor = static_cast<FollowAnchor>(m_followAnchorComboBox->currentIndex());
    config.follow.offsetMode = static_cast<FollowOffsetMode>(m_followOffsetModeComboBox->currentIndex());
    config.follow.offset = QPointF(m_followOffsetXSpinBox->value(), m_followOffsetYSpinBox->value());
    config.follow.pollIntervalMs = m_followPollIntervalSpinBox->value();
    config.follow.hideWhenMinimized = m_followHideMinimizedCheckBox->isChecked();

    if (config.follow.enabled) {
        config.isDesktopMode = false;
    }

    return config;
}

void MainWindow::refreshWindowList()
{
    m_followWindowComboBox->clear();
    QList<WindowInfo> windows = m_windowService.listWindows(true);
    if (windows.isEmpty()) {
        m_followWindowComboBox->addItem("没有可用窗口", QVariant::fromValue<qulonglong>(0));
        return;
    }

    for (const WindowInfo &info : windows) {
        QString title = info.title.trimmed();
        if (title.isEmpty()) {
            title = info.processName;
        }
        QString label = QString("%1 (%2)").arg(title, info.processName);
        m_followWindowComboBox->addItem(label, QVariant::fromValue<qulonglong>(info.handle));
    }
}

void MainWindow::updateFollowModeUi()
{
    bool followEnabled = m_followModeCheckBox->isChecked();
    bool batchEnabled = followEnabled && m_followBatchCheckBox->isChecked();

    if (followEnabled) {
        QSignalBlocker blocker(m_desktopModeCheckBox);
        m_desktopModeCheckBox->setChecked(false);
    }

    m_desktopModeCheckBox->setEnabled(!followEnabled);
    m_followWindowComboBox->setEnabled(followEnabled);
    m_refreshWindowsBtn->setEnabled(followEnabled);
    m_lockWindowBtn->setEnabled(followEnabled);
    m_followBatchCheckBox->setEnabled(followEnabled);
    m_followFilterTypeComboBox->setEnabled(batchEnabled);
    m_followFilterValueEdit->setEnabled(batchEnabled);
    m_followAnchorComboBox->setEnabled(followEnabled);
    m_followOffsetModeComboBox->setEnabled(followEnabled);
    m_followOffsetXSpinBox->setEnabled(followEnabled);
    m_followOffsetYSpinBox->setEnabled(followEnabled);
    m_followPollIntervalSpinBox->setEnabled(followEnabled);
    m_followHideMinimizedCheckBox->setEnabled(followEnabled);
}

void MainWindow::updateEventTable()
{
    if (m_currentStickerId.isEmpty()) {
        m_eventTable->setRowCount(0);
        return;
    }

    if (findConfigIndex(m_currentStickerId) < 0 || m_currentConfig.id != m_currentStickerId) {
        return;
    }

    m_eventTable->setRowCount(m_currentConfig.events.size());

    for (int i = 0; i < m_currentConfig.events.size(); ++i) {
        const auto &event = m_currentConfig.events[i];

        m_eventTable->setItem(i, 0, new QTableWidgetItem(mouseTriggersToString(event.trigger)));
        m_eventTable->setItem(i, 1, new QTableWidgetItem(eventTypeToString(event.type)));
        m_eventTable->setItem(i, 2, new QTableWidgetItem(event.target));
        m_eventTable->setItem(i, 3, new QTableWidgetItem(event.parameters));

        QTableWidgetItem *enabledItem = new QTableWidgetItem();
        enabledItem->setCheckState(event.enabled ? Qt::Checked : Qt::Unchecked);
        m_eventTable->setItem(i, 4, enabledItem);
    }
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
