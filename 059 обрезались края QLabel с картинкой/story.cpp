обнаружил что у всех выводимых картинок обрезается верхний и нижний край.
при этом картинки перед отображением сжимаются функцией

void PRVW::resizePixmap(QPixmap &pixmap, int max_w, int max_h)
{
	CodeMarkers::to_do("разобраться почему resizePixmap слегка отрезает верхний край");
	// Получаем размеры исходного изображения
	int originalWidth = pixmap.width();
	int originalHeight = pixmap.height();

	if (originalWidth > max_w || originalHeight > max_h) // Проверяем, нужно ли изменять размер
	{
		// Вычисляем коэффициенты масштабирования
		float widthRatio = static_cast<float>(max_w) / originalWidth;
		float heightRatio = static_cast<float>(max_h) / originalHeight;
		float scaleRatio = std::min(widthRatio, heightRatio);

		// Вычисляем новые размеры с округлением
		int newWidth = static_cast<int>(originalWidth * scaleRatio + 0.5f);
		int newHeight = static_cast<int>(originalHeight * scaleRatio + 0.5f);

		// Убедимся, что новые размеры не превышают max_w и max_h
		newWidth = std::min(newWidth, max_w);
		newHeight = std::min(newHeight, max_h);

		// Масштабируем изображение и обновляем переданный QPixmap
		pixmap = pixmap.scaled(newWidth, newHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	//qdbg << originalWidth << originalHeight << max_w << max_h << pixmap.width() << pixmap.height();

	//			320					180			302		 170		302					170

																//	302					153
}

перепроверил функцию, все нормально, продолжил расследование:
добавил дебаг вывод
	//qdbg << originalWidth << originalHeight << max_w << max_h << pixmap.width() << pixmap.height();
	//			320					180			302		 170		302					170
все нормально
добавил вывод размеров самого лейбла и оказалось 
QTimer::singleShot(0,[this](){ qdbg << labelPictOfFilm->height(); });
																//					height 153
начал искать, почему уменьен размер лейбла
размер лейбла ограничен панелью где он выводится
	QWidget *wForHlo3 = new QWidget;
	wForHlo3->setContentsMargins(0,0,0,0); // установи
	vlo_main->addWidget(wForHlo3);
	QHBoxLayout *hlo3 = new QHBoxLayout(wForHlo3);
	wForHlo3->setMaximumHeight(Sizes::heightHlo3); // ограничение высоты панели
	
	Sizes::heightHlo3 = 175

и высота сжатия картинки ставится 
maxHeightPrvwOfCurrent = heightHlo3 - 5;

я установил 0 ContentsMargins для панели
wForHlo3->setContentsMargins(0,0,0,0);

но забыл для тоже для hlo3
hlo3->setContentsMargins(0,0,0,0);

вот так перестал обрезать

	QWidget *wForHlo3 = new QWidget;
	wForHlo3->setContentsMargins(0,0,0,0);
	vlo_main->addWidget(wForHlo3);
	QHBoxLayout *hlo3 = new QHBoxLayout(wForHlo3);
	hlo3->setContentsMargins(0,0,0,0);
	wForHlo3->setMaximumHeight(Sizes::heightHlo3);
	
и даже внешний вид панели стал лучше