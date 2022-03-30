#include "gd_timelinefx.h"

#include "TLFXPugiXMLLoader.h"

#include "vogl_miniz_zip.h"

bool QtImage::Load()
{
	return true;
}

QtEffectsLibrary::QtEffectsLibrary() : _atlas(0)
{
	if (qApp == 0)
		qWarning() << "[QtEffectsLibrary] Application is not initialized.";
	else
		SetUpdateFrequency(qApp->primaryScreen()->refreshRate());

	_atlas = new QAtlasManager;
}

QtEffectsLibrary::~QtEffectsLibrary()
{
	delete _atlas;
}

bool QtEffectsLibrary::LoadLibrary(const char *library, const char *filename /* = 0 */, bool compile /* = true */)
{
	QString libraryinfo = filename;

	struct zip_archive_t {
		zip_archive_t() { memset(&za, 0, sizeof(mz_zip_archive)); }
		~zip_archive_t() { mz_zip_reader_end(&za); }
		mz_bool init_file(const char *fn) { return mz_zip_reader_init_file(&za, fn); }
		mz_zip_archive * operator &() { return &za; }
		mz_zip_archive za;
	} zip_archive;

	// Now try to open the archive.
	mz_bool status = zip_archive.init_file(library);
	if (!status)
	{
		qWarning() << "[QtEffectsLibrary] Cannot open effects library" << library;
		return false;
	}

	if (libraryinfo.isEmpty())
	{
		// Try to locate effect data file.
		for (int i = 0; i < (int)mz_zip_get_num_files(&zip_archive); i++)
		{
			mz_zip_archive_file_stat file_stat;
			if (!mz_zip_file_stat(&zip_archive, i, &file_stat))
			{
				qWarning() << "[QtEffectsLibrary] Cannot read effects library!";
				return false;
			}
			if(libraryinfo.isEmpty() && strcasestr(file_stat.m_filename, "data.xml"))
			{
				libraryinfo = file_stat.m_filename;
				break;
			}
			// qDebug("Filename: \"%s\", Comment: \"%s\", Uncompressed size: %u, Compressed size: %u, Is Dir: %u\n", file_stat.m_filename, file_stat.m_comment, (uint)file_stat.m_uncomp_size, (uint)file_stat.m_comp_size, mz_zip_is_file_a_directory(&zip_archive, i));
		}
	}

	if (libraryinfo.isEmpty())
	{
		qWarning() << "[QtEffectsLibrary] Cannot find library description file!";
		return false;
	}

	// Keep library we are using for effects
	_library = library;

	return Load(libraryinfo.toUtf8().constData(), compile);
}

TLFX::XMLLoader* QtEffectsLibrary::CreateLoader() const
{
	return new TLFX::PugiXMLLoader(_library.isEmpty()?0:_library.toUtf8().constData());
}

TLFX::AnimImage* QtEffectsLibrary::CreateImage() const
{
	return new QtImage();
}

bool QtEffectsLibrary::ensureTextureSize(int &w, int &h)
{
	// for texture bigger then atlas fix the size to textureLimit:
	if (w > _atlas->atlasTextureSize().width() || h > _atlas->atlasTextureSize().height())
	{
		qreal scale = qMin( _atlas->atlasTextureSizeLimit()/qreal(w),
							_atlas->atlasTextureSizeLimit()/qreal(h) );
		w *= scale; h *= scale;
		return false; // donot scale - keep size
	// if greater than atlas limit, make the size scaled :
	} else if (w > _atlas->atlasTextureSizeLimit() || h > _atlas->atlasTextureSizeLimit())
	{
		qreal scale = qMin( _atlas->atlasTextureSizeLimit()/qreal(w),
							_atlas->atlasTextureSizeLimit()/qreal(h) );
		w *= scale; h *= scale;
	}
	return true; // scale
}

bool QtEffectsLibrary::UploadTextures()
{
	// try calculate best fit into current atlas texture:
	int minw = 0, maxw = 0, minh = 0, maxh = 0;
	Q_FOREACH(TLFX::AnimImage *shape, _shapeList)
	{
		const int w = shape->GetWidth();
		const int h = shape->GetHeight();
		if (w < minw) minw = w;
		if (h < minh) minh = h;
		if (w > maxw) maxw = w;
		if (h > maxh) maxh = h;
	}
#define SC(x) (sc/(1.1+(x-minw)/(maxw-minw))) // 1-2
	qreal sc = 1.5; bool done = false; while(!done && sc > 0)
	{
		QGL::QAreaAllocator m_allocator(_atlas->atlasTextureSize(), _atlas->padding);
		qDebug() << "[QtEffectsLibrary] Scaling texture atlas with" << sc;
		done = true; Q_FOREACH(TLFX::AnimImage *shape, _shapeList)
		{
			const int anim_size = powf(2, ceilf(log2f(shape->GetFramesCount())));
			const int anim_square = sqrtf(anim_size);
			int w = shape->GetWidth()*anim_square;
			int h = shape->GetHeight()*anim_square;
			if(ensureTextureSize(w, h))
			{
				w *= SC(w);
				h *= SC(h);
			}
			QRect rc = m_allocator.allocate(QSize(w, h));
			if (rc.width() == 0 || rc.height() == 0)
			{
				sc -= 0.05; done = false; break; // next step
			}
		}
	}
	if (!done) {
		qDebug() << "[QtEffectsLibrary] Cannot build texture atlas.";
		return false;
	}
	if (!_library.isEmpty())
	{
		struct zip_archive_t {
			zip_archive_t() { memset(&za, 0, sizeof(mz_zip_archive)); }
			~zip_archive_t() { mz_zip_reader_end(&za); }
			mz_bool init_file(const char *fn) { return mz_zip_reader_init_file(&za, fn); }
			mz_zip_archive * operator &() { return &za; }
			mz_zip_archive za;
		} zip_archive;

		// Now try to open the archive.
		mz_bool status = zip_archive.init_file(_library.toUtf8().constData());
		if (status)
		{
			Q_FOREACH(TLFX::AnimImage *shape, _shapeList)
			{
				const char *filename = shape->GetFilename();
				const int anim_size = powf(2, ceilf(log2f(shape->GetFramesCount())));
				const int anim_square = sqrtf(anim_size);
				int w = shape->GetWidth()*anim_square;
				int h = shape->GetHeight()*anim_square;
				if(ensureTextureSize(w, h))
				{
					w *= SC(w);
					h *= SC(h);
				}

				if (filename==0 || strlen(filename)==0)
				{
					qWarning() << "[QtEffectsLibrary] Empty image filename";
					continue;
				}
				// Try to extract all the files to the heap.
				QStringList variants;
				variants
					<< filename
					<< QFileInfo(filename).fileName()
					<< QFileInfo(QString(filename).replace("\\","/")).fileName();
				Q_FOREACH(QString fn, variants)
				{
					size_t uncomp_size;
					void *p = mz_zip_extract_file_to_heap(&zip_archive, fn.toUtf8().constData(), &uncomp_size, 0);
					if (p == 0)
					{
						continue; // Try next name
					}

					qDebug() << "[QtEffectsLibrary] Successfully extracted file" << fn << uncomp_size << "bytes";

					QImage img = QImage::fromData((const uchar *)p, uncomp_size);

					if (img.isNull())
					{
						qWarning() << "[QtEffectsLibrary] Failed to create image:" << filename;
						return false;
					}
					else
					{
						switch (shape->GetImportOpt()) {
							case QtImage::impGreyScale:  __toGray2(img); break;
							case QtImage::impFullColour: break;
							case QtImage::impPassThrough: break;
							default: break;
						}

						// scale images to fit atlas
						QTexture *texture = _atlas->create(img.scaled(QSize(w,h)));
						dynamic_cast<QtImage*>(shape)->SetTexture(texture, filename);
						if (texture == 0) {
							qWarning() << "[QtEffectsLibrary] Failed to create texture for image" << filename << img.size() << QString("%1 frames").arg(shape->GetFramesCount());
							return false;
						} else
							break;
					}
					// We're done.
					mz_free(p);
				}
				if (0 == dynamic_cast<QtImage*>(shape)->GetTexture())
				{
					qWarning() << "[QtEffectsLibrary] Failed to extract file" << filename;
					return false;
				}
			}
			return true;
		} else {
			qWarning() << "[QtEffectsLibrary] Cannot open library file" << _library;
			return false;
		}
	} else {
		Q_FOREACH(TLFX::AnimImage *shape, _shapeList)
		{
			const char *filename = shape->GetFilename();
			const int anim_size = powf(2, ceilf(log2f(shape->GetFramesCount())));
			const int anim_square = sqrtf(anim_size);
			int w = shape->GetWidth()*anim_square;
			int h = shape->GetHeight()*anim_square;
			if(ensureTextureSize(w, h))
			{
				w *= SC(w);
				h *= SC(h);
			}

			QFile f(filename);
			if (!f.exists())
				f.setFileName(QString(":/data/%1").arg(filename));
			if (!f.exists()) {
				qWarning() << "[QtImage] Failed to load image:" << filename;
				return false;
			}
			QImage img(f.fileName());
			if (img.isNull())
			{
				qWarning() << "[QtImage] Failed to load image:" << filename;
				return false;
			}
			else
			{
				switch (shape->GetImportOpt()) {
					case QtImage::impGreyScale:  __toGray2(img); break;
					case QtImage::impFullColour: break;
					case QtImage::impPassThrough: break;
					default: break;
				}

				// scale images to fit atlas
				QTexture *texture = _atlas->create(img.scaled(QSize(w,h)));
				dynamic_cast<QtImage*>(shape)->SetTexture(texture, filename);
				if (texture == 0) {
					qWarning() << "[QtImage] Failed to create texture for image" << filename << img.size() << QString("%1 frames").arg(shape->GetFramesCount());
					return false;
				}
			}
		}
	}
	return true;
}

void QtEffectsLibrary::Debug(QGLPainter *p)
{
	Q_FOREACH(TLFX::AnimImage *sprite, _shapeList)
	{
		if (sprite->GetFramesCount() != 64) continue;

		// draw texture quad:

		glDisable( GL_DEPTH );
		glEnable( GL_BLEND );
		glDisable( GL_ALPHA_TEST );
		glEnable( GL_TEXTURE_2D );
		dynamic_cast<QtImage*>(sprite)->GetTexture()->bind();
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		static qreal f = 0;

		QRectF rc = dynamic_cast<QtImage*>(sprite)->GetTexture()->normalizedTextureSubRect();
		const int anim_size = powf(2, ceilf(log2f(sprite->GetFramesCount())));
		const int anim_square = sqrtf(anim_size);
		const int anim_frame = int(round(f)) % sprite->GetFramesCount(); f += 0.1;
		const int gr = anim_frame / anim_square, gc = anim_frame % anim_square;
		const float cw = rc.width()/anim_square, ch = rc.height()/anim_square;
		rc = QRectF(rc.x()+gc*cw, rc.y()+gr*ch, cw, ch);

		QGeometryData batch;
		batch.appendVertex(QVector3D(0,0,0));
		batch.appendVertex(QVector3D(sprite->GetWidth(),0,0));
		batch.appendVertex(QVector3D(sprite->GetWidth(),sprite->GetHeight(),0));
		batch.appendVertex(QVector3D(0,sprite->GetHeight(),0));
		batch.appendTexCoord(QVector2D(rc.x(), rc.y()));
		batch.appendTexCoord(QVector2D(rc.x()+rc.width(), rc.y()));
		batch.appendTexCoord(QVector2D(rc.x()+rc.width(), rc.y()+rc.height()));
		batch.appendTexCoord(QVector2D(rc.x(), rc.y()+rc.height()));
		batch.appendColor(Qt::white);
		batch.appendColor(Qt::white);
		batch.appendColor(Qt::white);
		batch.appendColor(Qt::white);
		batch.appendIndices(0,1,2);
		batch.appendIndices(2,3,0);
		batch.draw(p,0,6,QGL::Triangles);
		dynamic_cast<QtImage*>(sprite)->GetTexture()->release();

		return;
	}
}

QtParticleManager::QtParticleManager( QGLPainter *p, int particles /*= particleLimit*/, int layers /*= 1*/ )
	: TLFX::ParticleManager(particles, layers)
	, _lastTexture(0)
	, _lastAdditive(true), _globalBlend(FromEffectBlendMode)
	, _p(p)
{
}

static void __build_tiles(QSize grid_size, unsigned int total_frames, 
						QPointF tex_origin=QPointF(0,0), QSizeF tex_size=QSizeF(1,1))
{
	QVector<QRectF> frames;
	for(int fr=0; fr<grid_size.height() /* rows */; fr++)
		for(int fc=0; fc<grid_size.width() /* cols */; fc++) {
			const float cw = tex_size.width()/grid_size.width(), ch = tex_size.height()/grid_size.height();
			frames.push_back(QRectF(tex_origin.x()+fc*cw, tex_origin.y()+fr*ch, cw, ch));
			if (frames.size() == total_frames) break;
		}
}
	
void QtParticleManager::DrawSprite( TLFX::Particle *p, TLFX::AnimImage* sprite, float px, float py, float frame, float x, float y, float rotation, float scaleX, float scaleY, unsigned char r, unsigned char g, unsigned char b, float a , bool additive )
{
	#define qFF(C) C*(255.999)

	quint8 alpha = qFF(a);
	if (alpha == 0 || scaleX == 0 || scaleY == 0) return;

	if ((_lastTexture && dynamic_cast<QtImage*>(sprite)->GetTexture()->textureId() != _lastTexture->textureId())
		|| (additive != _lastAdditive))
		Flush();

	QRectF rc = dynamic_cast<QtImage*>(sprite)->GetTexture()->normalizedTextureSubRect();
	// calculate frame position in atlas:
	if (sprite && sprite->GetFramesCount() > 1)
	{
		const int anim_size = powf(2, ceilf(log2f(sprite->GetFramesCount())));
		const int anim_square = sqrtf(anim_size);
		const int anim_frame = floorf(frame);
		if(anim_frame >= sprite->GetFramesCount()) {
			qWarning() << "[QtParticleManager] Out of range:" << frame << anim_frame << "frames:" << sprite->GetFramesCount();
		}
		const int gr = anim_frame / anim_square, gc = anim_frame % anim_square;
		const float cw = rc.width()/anim_square, ch = rc.height()/anim_square;
		rc = QRectF(rc.x()+gc*cw, rc.y()+gr*ch, cw, ch);
		//qDebug() << sprite->GetFilename() << anim_frame << dynamic_cast<QtImage*>(sprite)->GetTexture()->normalizedTextureSubRect() << rc;
	}

	//uvs[index + 0] = {0, 0};
	batch.appendTexCoord(QVector2D(rc.x(), rc.y()));
	//uvs[index + 1] = {1.0f, 0}
	batch.appendTexCoord(QVector2D(rc.x()+rc.width(), rc.y()));
	//uvs[index + 2] = {1.0f, 1.0f};
	batch.appendTexCoord(QVector2D(rc.x()+rc.width(), rc.y()+rc.height()));
	//uvs[index + 3] = {0, 1.0f};
	batch.appendTexCoord(QVector2D(rc.x(), rc.y()+rc.height()));

	/*
	verts[index + 0].x = px - x * scaleX;
	verts[index + 0].y = py - y * scaleY;
	//verts[index + 0].z = 1.0f;
	verts[index + 1].x = verts[index + 0].x + _lastSprite->GetWidth() * scaleX;
	verts[index + 1].y = verts[index + 0].y;
	//verts[index + 1].z = 1.0f;
	verts[index + 2].x = verts[index + 1].x;
	verts[index + 2].y = verts[index + 1].y + _lastSprite->GetHeight() * scaleY;
	//verts[index + 2].z = 1.0f;
	verts[index + 3].x = verts[index + 0].x;
	verts[index + 3].y = verts[index + 2].y;
	//verts[index + 3].z = 1.0f;
	*/

	float x0 = -x * scaleX;
	float y0 = -y * scaleY;
	float x1 = x0;
	float y1 = (-y + sprite->GetHeight()) * scaleY;
	float x2 = (-x + sprite->GetWidth()) * scaleX;
	float y2 = y1;
	float x3 = x2;
	float y3 = y0;

	float cos = cosf(rotation / 180.f * M_PI);
	float sin = sinf(rotation / 180.f * M_PI);

	//verts[index + 0] = {px + x0 * cos - y0 * sin, py + x0 * sin + y0 * cos};
	//verts[index + 0].z = 1.0f;
	batch.appendVertex(QVector3D(px + x0 * cos - y0 * sin, py + x0 * sin + y0 * cos, 0));
	//verts[index + 1] = {px + x1 * cos - y1 * sin, py + x1 * sin + y1 * cos};
	//verts[index + 1].z = 1.0f;
	batch.appendVertex(QVector3D(px + x1 * cos - y1 * sin, py + x1 * sin + y1 * cos, 0));
	//verts[index + 2] = {px + x2 * cos - y2 * sin, py + x2 * sin + y2 * cos};
	//verts[index + 2].z = 1.0f;
	batch.appendVertex(QVector3D(px + x2 * cos - y2 * sin, py + x2 * sin + y2 * cos, 0));
	//verts[index + 3] = {px + x3 * cos - y3 * sin, py + x3 * sin + y3 * cos};
	//verts[index + 3].z = 1.0f;
	batch.appendVertex(QVector3D(px + x3 * cos - y3 * sin, py + x3 * sin + y3 * cos, 0));

	for (int i = 0; i < 4; ++i)
	{
		batch.appendColor(QColor(r, g, b, alpha));
	}

	_lastTexture = dynamic_cast<QtImage*>(sprite)->GetTexture();
	switch(_globalBlend)
	{
		case FromEffectBlendMode: _lastAdditive = additive; break;
		case AddBlendMode: _lastAdditive = true; break;
		case AlphaBlendMode: _lastAdditive = false; break;
	}
}

void QtParticleManager::Flush()
{
	if (batch.count())
	{
		Q_ASSERT(_p);

		glDisable( GL_DEPTH );
		glEnable( GL_BLEND );
		glDisable( GL_ALPHA_TEST );
		if (_lastTexture) {
			glEnable( GL_TEXTURE_2D );
			_lastTexture->bind();
		} else
			glDisable( GL_TEXTURE_2D );
		if (_lastAdditive) {
			// ALPHA_ADD
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
		} else {
			// ALPHA_BLEND
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}

		QGLBuilder builder;
		builder.addQuads(batch);
		QList<QGeometryData> opt = builder.optimized();
		Q_FOREACH(QGeometryData gd, opt) {
			gd.draw(_p, 0, gd.indexCount());
		}
		if(_lastTexture)
			_lastTexture->release();
		batch = QGeometryData(); // clear batch data
	}
}
