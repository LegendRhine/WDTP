# Write Down, Then Publish (WDTP)
 
### 系统设置
- PropertiesFile (systemFile, 系统属性文件)位于本机“用户-文档”目录下，文件名为：wdtp.sys，属性如下：
	- recentFiles：最近打开的10个项目
	- language: 界面语言（字符串，语言名称）
	- uiBackground: 界面背景色
	- uiTextColour: 界面文字只有深浅两种颜色，根据UI颜色的亮度确定
	- fontName: 文本编辑器所用的字体(暂未使用)
	- fontSize：文本编辑器的字体大小
	- editorFontColour: 文本编辑器文字颜色
	- editorBackground: 文本编辑器背景色

### 项目文档
- 项目文档（*.wdtp）采用ValueTree数据模型+TreeView视图+派生的TreeViewItem实现文档的组织管理、显示与交互等功能。
- 项目文档的内部结构（值树结构）同磁盘目录结构，但不含每个磁盘目录下的media文件夹，也不含'docs'之外的任何磁盘文件夹

### 属性名
- wdtpProject总值树的属性（项目属性）：
    - name: 值为“site”
    - tile: 项目名称
	- keywords: 关键字
    - description: 简述
	- owner: 项目所有者（作者）
	- contact 对应模板文件中的{{contact}}
	- copyright: 显示在网页页底的版权信息
	- order: 文件树的排序方式。文件名（0）、标题/简介（1）、网页文件名（2）、文件大小（3）、创建时间（4）、修改时间（5）
	- dirFirst: 目录排在前面（0），还是文档排在前面（1）
	- ascending: 升序(0)还是降序(1)
	- showWhat: TreeView中显示文件名（0）、标题/简介（1），还是网页文件名（2）
	- tooltip：TreeView中的tooltip显示文件名（包括所有路径，0）、标题/简介（1），还是网页文件名（2）
	- render: 模板目录（磁盘Themes下的某个子目录，即已安装的某套模板）
	- tplFile: 渲染网站首页所使用的模板文件，位于render所设置的目录下
	- js: 首页所需的js代码
	- ad: 广告代码（任意html代码），对应模板文件中的标签{{ad}}
    - modifyDate: 最后一次修改的日期
	- needCreateHtml: 本站根目录下的index.html是否需要生成
	- identityOfLastSelectedItem: 文件树中最后一次选择的文档的标示，
	  可由TreeView的findItemFromIdentifierString()找到对应的item，
	  此值用于文件树中移动项目、打开项目时自动选择最后一次点选的item

- 值树中各个dir（目录）的属性：
    - name：目录名 (不包含任何上级路径。上级路径可通过其父节点的name获取）
	- title：标题，即网页上所显示的栏目名称
	- keywords: 关键字
    - description: 目录简述，用于index的description
	- createDate: 创建日期
    - modifyDate: 最后一次修改的日期
	- isMenu：是否为网站菜单
	- tplFile: 渲染目录index页所使用的模板文件，位于项目属性render所设置的目录下
	- js: 目录index页所需的js代码
	- needCreateHtml: 本目录下的index.html是否需要生成

- 值树中各个doc（文档）的属性：
    - name：文件名（不含文件扩展名，不包含任何上级路径。上级路径可通过其父节点的name获取）
	- title：文章或页面的标题
	- keywords: 关键字
    - description: 简述，用于index的description
	- createDate: 创建日期
    - modifyDate: 最后一次修改的日期
	- isMenu: 本文挡是否为网站菜单（不加入列表页。其模板则可以是articel，也可以是page）。注意：作为网站菜单的文档必须位于根目录下
	- tplFile: 渲染本文档所使用的模板文件，位于项目属性render所设置的目录下
	- js: 网页所需的js代码
	- thumb: bool值，目录index页面中是否提取并显示本文档中的第一幅图片（标题图）
	- thumbName: 标题图的文件名, 带有“media/”前缀（防止不带时无法定位其他目录media下的图像）
	- needCreateHtml: 本文档是否需要生成网页

### 模板标签
- MD文档生成html时，替换模板html中的“标签项”（格式为：{{xxxx}}），要替换的项目如下：
    - {{siteRelativeRootPath}} 该网页相对于网站根目录的路径，用于该网页链接网站根目录下的css样式表文件。
	  比如：当前网页的地址是：“site/dir/subDir/00.html”  则该值应该是：“../../../”。注意：最后一定是“/”
	  此标签可用于<head>区中连接外部css文件等情况

	- {{keywords}} <head>区meta属性中的的关键字
	- {{description}} <head>区meta属性中的描述，即：MD文档中有实际内容的第二段（标题后面的非空行段落）
	- {{title}} <head>区meta属性中的标题，即：MD文档的第一行（标题）

	- {{contentTitle}} <body>区文章或目录的标题
	- {{content}} <body>区中所显示的网页内容，即：MD文档的所有内容

	- {{siteLogo}} 显示网站LOGO图片，图片位于网站根目录add-in文件夹下，文件名为logo.png
	- {{siteMenu}} 网站主菜单
	- {{siteNavi}} 当前页面的导航菜单

	- {{createAndModifyTime}} 创作及最后修改时间
	- {{previousAndNext}} 上一篇，下一篇
	- {{ad}} 广告代码，在项目根目录中设置. 格式：“图像文件名 链接地址”。图像文件应位于site/add-in目录下
	- {{random}} 随机推荐5篇本站文章
	- {{contact}} 联系方式

	- {{toTop}} 点击后回到页面顶端
	- {{backPrevious}} 专供book模板的index页所用，返回上一级
	- {{bottomCopyright}} 每页页底的版权信息

	- {{titleOfDir}} <body>区中所显示的当前目录的标题
	- {{blogList}} 专供blog模板的index网页。文章列表，倒序（新的在上），不含目录，含描述，每页显示10条
	- {{bookList}} 专供book模板的index网页。目录与文章列表，按文件名排序，含目录，不含描述，一页显示全部

### 关于排序

- 项目文件中的条目(总植树)不进行排序
- 文件树面板中条目显示的顺序由 DocTreeViewItem 类内部完成（内存中排序）

