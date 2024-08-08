#include "AntTweakBar.h"

GLuint g_SmallFontTexID = 0;
GLuint g_NormalFontTexID = 0;
GLuint g_LargeFontTexID = 0;

static GLuint GLBindFont(const CTexFont *_Font) {
	GLuint TexID = 0;
	GL::_glGenTextures(1, &TexID);
	GL::_glBindTexture(GL_TEXTURE_2D, TexID);
	GL::_glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	GL::_glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
	GL::_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	GL::_glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	GL::_glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	GL::_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	GL::_glPixelTransferf(GL_ALPHA_SCALE, 1);
	GL::_glPixelTransferf(GL_ALPHA_BIAS, 0);
	GL::_glPixelTransferf(GL_RED_BIAS, 1);
	GL::_glPixelTransferf(GL_GREEN_BIAS, 1);
	GL::_glPixelTransferf(GL_BLUE_BIAS, 1);
	GL::_glTexImage2D(GL_TEXTURE_2D, 0, 4, _Font->m_TexWidth, _Font->m_TexHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, _Font->m_TexBytes);
	GL::_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	GL::_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	GL::_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GL::_glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GL::_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	GL::_glBindTexture(GL_TEXTURE_2D, 0);
	GL::_glPixelTransferf(GL_ALPHA_BIAS, 0);
	GL::_glPixelTransferf(GL_RED_BIAS, 0);
	GL::_glPixelTransferf(GL_GREEN_BIAS, 0);
	GL::_glPixelTransferf(GL_BLUE_BIAS, 0);

	return TexID;
}

int CTwGraphOpenGL::Init() {
	m_Drawing = false;
	m_FontTexID = 0;
	m_FontTex = NULL;
	m_MaxClipPlanes = -1;

	if (LoadOpenGL() == 0) {
		g_TwMgr->SetLastError(g_ErrCantLoadOGL);
		return 0;
	}

	m_SupportTexRect = false; // updated in BeginDraw

	return 1;
}

//  ---------------------------------------------------------------------------

int CTwGraphOpenGL::Shut() {
	assert(m_Drawing == false);

	GLUnbindFont(m_FontTexID);

	int Res = 1;
	if (UnloadOpenGL() == 0) {
		g_TwMgr->SetLastError(g_ErrCantUnloadOGL);
		Res = 0;
	}

	return Res;
}

void CTwGraphOpenGL::BeginDraw(int _WndWidth, int _WndHeight) {
	assert(m_Drawing == false && _WndWidth > 0 && _WndHeight > 0);
	m_Drawing = true;
	m_WndWidth = _WndWidth;
	m_WndHeight = _WndHeight;

	CHECK_GL_ERROR;

	//#if !defined(ANT_OSX)
	static bool s_SupportTexRectChecked = false;
	if (!s_SupportTexRectChecked) {
		const char *ext = (const char *)GL::_glGetString(GL_EXTENSIONS);
		if (ext != 0 && strlen(ext) > 0)
			m_SupportTexRect = (strstr(ext, "GL_ARB_texture_rectangle") != NULL);
		s_SupportTexRectChecked = true;
	}
	//#endif

	GL::_glPushAttrib(GL_ALL_ATTRIB_BITS);
	GL::_glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	if (GL::_glActiveTextureARB) {
		GL::_glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &m_PrevActiveTextureARB);
		GL::_glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE_ARB, &m_PrevClientActiveTextureARB);
		GLint maxTexUnits = 1;
		GL::_glGetIntegerv(GL_MAX_TEXTURE_COORDS, &maxTexUnits); // was GL_MAX_TEXTURE_UNITS_ARB
		if (maxTexUnits < 1)
			maxTexUnits = 1;
		else if (maxTexUnits > MAX_TEXTURES)
			maxTexUnits = MAX_TEXTURES;
		GLint i;
		for (i = 0; i < maxTexUnits; ++i) {
			GL::_glActiveTextureARB(GL_TEXTURE0_ARB + i);
			m_PrevActiveTexture1D[i] = GL::_glIsEnabled(GL_TEXTURE_1D);
			m_PrevActiveTexture2D[i] = GL::_glIsEnabled(GL_TEXTURE_2D);
			m_PrevActiveTexture3D[i] = GL::_glIsEnabled(GL_TEXTURE_3D);
			GL::_glDisable(GL_TEXTURE_1D);
			GL::_glDisable(GL_TEXTURE_2D);
			GL::_glDisable(GL_TEXTURE_3D);
		}
		GL::_glActiveTextureARB(GL_TEXTURE0_ARB);

		for (i = 0; i < maxTexUnits; i++) {
			GL::_glClientActiveTextureARB(GL_TEXTURE0_ARB + i);
			m_PrevClientTexCoordArray[i] = GL::_glIsEnabled(GL_TEXTURE_COORD_ARRAY);
			GL::_glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		GL::_glClientActiveTextureARB(GL_TEXTURE0_ARB);
	}

	GL::_glMatrixMode(GL_TEXTURE);
	GL::_glPushMatrix();
	GL::_glLoadIdentity();
	GL::_glMatrixMode(GL_MODELVIEW);
	GL::_glPushMatrix();
	GL::_glLoadIdentity();
	GL::_glMatrixMode(GL_PROJECTION);
	GL::_glPushMatrix();
	GLint Vp[4];
	GL::_glGetIntegerv(GL_VIEWPORT, Vp);
	/*
	if( _WndWidth>0 && _WndHeight>0 )
	{
		Vp[0] = 0;
		Vp[1] = 0;
		Vp[2] = _WndWidth;
		Vp[3] = _WndHeight;
		GL::_glViewport(Vp[0], Vp[1], Vp[2], Vp[3]);
	}
	GL::_glLoadIdentity();
	//GL::_glOrtho(Vp[0], Vp[0]+Vp[2]-1, Vp[1]+Vp[3]-1, Vp[1], -1, 1); // Doesn't work
	GL::_glOrtho(Vp[0], Vp[0]+Vp[2], Vp[1]+Vp[3], Vp[1], -1, 1);
	*/
	if (_WndWidth > 0 && _WndHeight > 0) {
		Vp[0] = 0;
		Vp[1] = 0;
		Vp[2] = _WndWidth - 1;
		Vp[3] = _WndHeight - 1;
		GL::_glViewport(Vp[0], Vp[1], Vp[2], Vp[3]);
	}
	GL::_glLoadIdentity();
	GL::_glOrtho(Vp[0], Vp[0] + Vp[2], Vp[1] + Vp[3], Vp[1], -1, 1);
	GL::_glGetIntegerv(GL_VIEWPORT, m_ViewportInit);
	GL::_glGetFloatv(GL_PROJECTION_MATRIX, m_ProjMatrixInit);

	GL::_glGetFloatv(GL_LINE_WIDTH, &m_PrevLineWidth);
	GL::_glDisable(GL_POLYGON_STIPPLE);
	GL::_glLineWidth(1);
	GL::_glDisable(GL_LINE_SMOOTH);
	GL::_glDisable(GL_LINE_STIPPLE);
	GL::_glDisable(GL_CULL_FACE);
	GL::_glDisable(GL_DEPTH_TEST);
	GL::_glDisable(GL_LIGHTING);
	GL::_glEnable(GL_BLEND);
	GL::_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL::_glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &m_PrevTexEnv);
	GL::_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	GL::_glGetIntegerv(GL_POLYGON_MODE, m_PrevPolygonMode);
	GL::_glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GL::_glDisable(GL_ALPHA_TEST);
	//GL::_glEnable(GL_ALPHA_TEST);
	//GL::_glAlphaFunc(GL_GREATER, 0);
	GL::_glDisable(GL_FOG);
	GL::_glDisable(GL_LOGIC_OP);
	GL::_glDisable(GL_SCISSOR_TEST);
	if (m_MaxClipPlanes < 0) {
		GL::_glGetIntegerv(GL_MAX_CLIP_PLANES, &m_MaxClipPlanes);
		if (m_MaxClipPlanes < 0 || m_MaxClipPlanes > 255)
			m_MaxClipPlanes = 6;
	}
	for (GLint i = 0; i < m_MaxClipPlanes; ++i)
		GL::_glDisable(GL_CLIP_PLANE0 + i);
	m_PrevTexture = 0;
	GL::_glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_PrevTexture);

	GL::_glDisableClientState(GL_VERTEX_ARRAY);
	GL::_glDisableClientState(GL_NORMAL_ARRAY);
	GL::_glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	GL::_glDisableClientState(GL_INDEX_ARRAY);
	GL::_glDisableClientState(GL_COLOR_ARRAY);
	GL::_glDisableClientState(GL_EDGE_FLAG_ARRAY);

	if (GL::_glBindVertexArray != NULL) {
		m_PrevVertexArray = 0;
		GL::_glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint *)&m_PrevVertexArray);
		GL::_glBindVertexArray(0);
	}
	if (GL::_glBindBufferARB != NULL) {
		m_PrevArrayBufferARB = m_PrevElementArrayBufferARB = 0;
		GL::_glGetIntegerv(GL_ARRAY_BUFFER_BINDING_ARB, &m_PrevArrayBufferARB);
		GL::_glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, &m_PrevElementArrayBufferARB);
		GL::_glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		GL::_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}
	if (GL::_glBindProgramARB != NULL) {
		m_PrevVertexProgramARB = GL::_glIsEnabled(GL_VERTEX_PROGRAM_ARB);
		m_PrevFragmentProgramARB = GL::_glIsEnabled(GL_FRAGMENT_PROGRAM_ARB);
		GL::_glDisable(GL_VERTEX_PROGRAM_ARB);
		GL::_glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
	if (GL::_glGetHandleARB != NULL && GL::_glUseProgramObjectARB != NULL) {
		m_PrevProgramObjectARB = GL::_glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
		GL::_glUseProgramObjectARB(0);
	}
	GL::_glDisable(GL_TEXTURE_1D);
	GL::_glDisable(GL_TEXTURE_2D);
	if (GL::_glTexImage3D != NULL) {
		m_PrevTexture3D = GL::_glIsEnabled(GL_TEXTURE_3D);
		GL::_glDisable(GL_TEXTURE_3D);
	}

	if (m_SupportTexRect) {
		m_PrevTexRectARB = GL::_glIsEnabled(GL_TEXTURE_RECTANGLE_ARB);
		GL::_glDisable(GL_TEXTURE_RECTANGLE_ARB);
	}
	if (GL::_glBlendEquationSeparate != NULL) {
		GL::_glGetIntegerv(GL_BLEND_EQUATION_RGB, &m_PrevBlendEquationRGB);
		GL::_glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &m_PrevBlendEquationAlpha);
		GL::_glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	}
	if (GL::_glBlendFuncSeparate != NULL) {
		GL::_glGetIntegerv(GL_BLEND_SRC_RGB, &m_PrevBlendSrcRGB);
		GL::_glGetIntegerv(GL_BLEND_DST_RGB, &m_PrevBlendDstRGB);
		GL::_glGetIntegerv(GL_BLEND_SRC_ALPHA, &m_PrevBlendSrcAlpha);
		GL::_glGetIntegerv(GL_BLEND_DST_ALPHA, &m_PrevBlendDstAlpha);
		GL::_glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	if (GL::_glBlendEquation != NULL) {
		GL::_glGetIntegerv(GL_BLEND_EQUATION, &m_PrevBlendEquation);
		GL::_glBlendEquation(GL_FUNC_ADD);
	}
	if (GL::_glDisableVertexAttribArray != NULL) {
		GLint maxVertexAttribs;
		GL::_glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
		if (maxVertexAttribs > MAX_VERTEX_ATTRIBS)
			maxVertexAttribs = MAX_VERTEX_ATTRIBS;

		for (int i = 0; i < maxVertexAttribs; i++) {
			GL::_glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &m_PrevEnabledVertexAttrib[i]);
			GL::_glDisableVertexAttribArray(i);
		}
	}

	CHECK_GL_ERROR;
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::EndDraw() {
	assert(m_Drawing == true);
	m_Drawing = false;

	GL::_glBindTexture(GL_TEXTURE_2D, m_PrevTexture);
	if (GL::_glBindVertexArray != NULL)
		GL::_glBindVertexArray(m_PrevVertexArray);
	if (GL::_glBindBufferARB != NULL) {
		GL::_glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_PrevArrayBufferARB);
		GL::_glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_PrevElementArrayBufferARB);
	}
	if (GL::_glBindProgramARB != NULL) {
		if (m_PrevVertexProgramARB)
			GL::_glEnable(GL_VERTEX_PROGRAM_ARB);
		if (m_PrevFragmentProgramARB)
			GL::_glEnable(GL_FRAGMENT_PROGRAM_ARB);
	}
	if (GL::_glGetHandleARB != NULL && GL::_glUseProgramObjectARB != NULL)
		GL::_glUseProgramObjectARB(m_PrevProgramObjectARB);
	if (GL::_glTexImage3D != NULL && m_PrevTexture3D)
		GL::_glEnable(GL_TEXTURE_3D);
	if (m_SupportTexRect && m_PrevTexRectARB)
		GL::_glEnable(GL_TEXTURE_RECTANGLE_ARB);
	if (GL::_glBlendEquation != NULL)
		GL::_glBlendEquation(m_PrevBlendEquation);
	if (GL::_glBlendEquationSeparate != NULL)
		GL::_glBlendEquationSeparate(m_PrevBlendEquationRGB, m_PrevBlendEquationAlpha);
	if (GL::_glBlendFuncSeparate != NULL)
		GL::_glBlendFuncSeparate(m_PrevBlendSrcRGB, m_PrevBlendDstRGB, m_PrevBlendSrcAlpha, m_PrevBlendDstAlpha);

	GL::_glPolygonMode(GL_FRONT, m_PrevPolygonMode[0]);
	GL::_glPolygonMode(GL_BACK, m_PrevPolygonMode[1]);
	GL::_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_PrevTexEnv);
	GL::_glLineWidth(m_PrevLineWidth);
	GL::_glMatrixMode(GL_PROJECTION);
	GL::_glPopMatrix();
	GL::_glMatrixMode(GL_MODELVIEW);
	GL::_glPopMatrix();
	GL::_glMatrixMode(GL_TEXTURE);
	GL::_glPopMatrix();
	GL::_glPopClientAttrib();
	GL::_glPopAttrib();

	if (GL::_glActiveTextureARB) {
		GLint maxTexUnits = 1;
		GL::_glGetIntegerv(GL_MAX_TEXTURE_COORDS, &maxTexUnits); // was GL_MAX_TEXTURE_UNITS_ARB
		if (maxTexUnits < 1)
			maxTexUnits = 1;
		else if (maxTexUnits > MAX_TEXTURES)
			maxTexUnits = MAX_TEXTURES;
		GLint i;
		for (i = 0; i < maxTexUnits; ++i) {
			GL::_glActiveTextureARB(GL_TEXTURE0_ARB + i);
			if (m_PrevActiveTexture1D[i])
				GL::_glEnable(GL_TEXTURE_1D);
			if (m_PrevActiveTexture2D[i])
				GL::_glEnable(GL_TEXTURE_2D);
			if (m_PrevActiveTexture3D[i])
				GL::_glEnable(GL_TEXTURE_3D);
		}
		GL::_glActiveTextureARB(m_PrevActiveTextureARB);

		for (i = 0; i < maxTexUnits; ++i) {
			GL::_glClientActiveTextureARB(GL_TEXTURE0_ARB + i);
			if (m_PrevClientTexCoordArray[i])
				GL::_glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		GL::_glClientActiveTextureARB(m_PrevClientActiveTextureARB);
	}
	if (GL::_glEnableVertexAttribArray) {
		GLint maxVertexAttribs;
		GL::_glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
		if (maxVertexAttribs > MAX_VERTEX_ATTRIBS)
			maxVertexAttribs = MAX_VERTEX_ATTRIBS;

		for (int i = 0; i < maxVertexAttribs; i++) {
			if (m_PrevEnabledVertexAttrib[i] != 0)
				GL::_glEnableVertexAttribArray(i);
		}
	}

	CHECK_GL_ERROR;
}

//  ---------------------------------------------------------------------------

bool CTwGraphOpenGL::IsDrawing() {
	return m_Drawing;
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::Restore() {
	GLUnbindFont(m_FontTexID);
	m_FontTexID = 0;
	m_FontTex = NULL;
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::DrawLine(int _X0, int _Y0, int _X1, int _Y1, color32 _Color0, color32 _Color1, bool _AntiAliased) {
	assert(m_Drawing == true);
	/*
	// border adjustment NO!!
	if(_X0<_X1)
		++_X1;
	else if(_X0>_X1)
		++_X0;
	if(_Y0<_Y1)
		++_Y1;
	else if(_Y0>_Y1)
		++_Y0;
	*/
	//const GLfloat dx = +0.0f;
	const GLfloat dx = +0.5f;
	//GLfloat dy = -0.2f;
	const GLfloat dy = -0.5f;
	if (_AntiAliased)
		GL::_glEnable(GL_LINE_SMOOTH);
	else
		GL::_glDisable(GL_LINE_SMOOTH);
	GL::_glDisable(GL_TEXTURE_2D);
	GL::_glMatrixMode(GL_MODELVIEW);
	GL::_glLoadIdentity();
	GL::_glBegin(GL_LINES);
	GL::_glColor4ub(GLubyte(_Color0 >> 16), GLubyte(_Color0 >> 8), GLubyte(_Color0), GLubyte(_Color0 >> 24));
	GL::_glVertex2f((GLfloat)_X0 + dx, (GLfloat)_Y0 + dy);
	GL::_glColor4ub(GLubyte(_Color1 >> 16), GLubyte(_Color1 >> 8), GLubyte(_Color1), GLubyte(_Color1 >> 24));
	GL::_glVertex2f((GLfloat)_X1 + dx, (GLfloat)_Y1 + dy);
	//GL::_glVertex2i(_X0, _Y0);
	//GL::_glVertex2i(_X1, _Y1);
	GL::_glEnd();
	GL::_glDisable(GL_LINE_SMOOTH);
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::DrawRect(int _X0, int _Y0, int _X1, int _Y1, color32 _Color00, color32 _Color10, color32 _Color01, color32 _Color11) {
	assert(m_Drawing == true);

	/*
	// border adjustment
	if(_X0<_X1)
		++_X1;
	else if(_X0>_X1)
		++_X0;
	if(_Y0<_Y1)
		++_Y1;
	else if(_Y0>_Y1)
		++_Y0;
	*/
	// border adjustment
	if (_X0 < _X1)
		++_X1;
	else if (_X0 > _X1)
		++_X0;
	if (_Y0 < _Y1)
		--_Y0;
	else if (_Y0 > _Y1)
		--_Y1;
	const GLfloat dx = +0.0f;
	const GLfloat dy = +0.0f;

	GL::_glDisable(GL_TEXTURE_2D);
	GL::_glMatrixMode(GL_MODELVIEW);
	GL::_glLoadIdentity();
	//GLubyte r = GLubyte(_Color>>16);
	//GLubyte g = GLubyte(_Color>>8);
	//GLubyte b = GLubyte(_Color);
	//GLubyte a = GLubyte(_Color>>24);
	//GL::_glColor4ub(GLubyte(_Color>>16), GLubyte(_Color>>8), GLubyte(_Color), GLubyte(_Color>>24));
	//GL::_glColor4ub(r, g, b, a);
	GL::_glBegin(GL_QUADS);
	GL::_glColor4ub(GLubyte(_Color00 >> 16), GLubyte(_Color00 >> 8), GLubyte(_Color00), GLubyte(_Color00 >> 24));
	GL::_glVertex2f((GLfloat)_X0 + dx, (GLfloat)_Y0 + dy);
	GL::_glColor4ub(GLubyte(_Color10 >> 16), GLubyte(_Color10 >> 8), GLubyte(_Color10), GLubyte(_Color10 >> 24));
	GL::_glVertex2f((GLfloat)_X1 + dx, (GLfloat)_Y0 + dy);
	GL::_glColor4ub(GLubyte(_Color11 >> 16), GLubyte(_Color11 >> 8), GLubyte(_Color11), GLubyte(_Color11 >> 24));
	GL::_glVertex2f((GLfloat)_X1 + dx, (GLfloat)_Y1 + dy);
	GL::_glColor4ub(GLubyte(_Color01 >> 16), GLubyte(_Color01 >> 8), GLubyte(_Color01), GLubyte(_Color01 >> 24));
	GL::_glVertex2f((GLfloat)_X0 + dx, (GLfloat)_Y1 + dy);
	GL::_glEnd();
}

//  ---------------------------------------------------------------------------

void *CTwGraphOpenGL::NewTextObj() {
	return new CTextObj;
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::DeleteTextObj(void *_TextObj) {
	assert(_TextObj != NULL);
	delete static_cast<CTextObj *>(_TextObj);
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::BuildText(void *_TextObj, const std::string *_TextLines, color32 *_LineColors, color32 *_LineBgColors, int _NbLines, const CTexFont *_Font, int _Sep, int _BgWidth) {
	assert(m_Drawing == true);
	assert(_TextObj != NULL);
	assert(_Font != NULL);

	if (_Font != m_FontTex) {
		GLUnbindFont(m_FontTexID);
		m_FontTexID = GLBindFont(_Font);
		m_FontTex = _Font;
	}
	CTextObj *TextObj = static_cast<CTextObj *>(_TextObj);
	TextObj->m_TextVerts.resize(0);
	TextObj->m_TextUVs.resize(0);
	TextObj->m_BgVerts.resize(0);
	TextObj->m_Colors.resize(0);
	TextObj->m_BgColors.resize(0);

	int x, x1, y, y1, i, Len;
	unsigned char ch;
	const unsigned char *Text;
	color32 LineColor = COLOR32_RED;
	for (int Line = 0; Line < _NbLines; ++Line) {
		x = 0;
		y = Line * (_Font->m_CharHeight + _Sep);
		y1 = y + _Font->m_CharHeight;
		Len = (int)_TextLines[Line].length();
		Text = (const unsigned char *)(_TextLines[Line].c_str());
		if (_LineColors != NULL)
			LineColor = (_LineColors[Line] & 0xff00ff00) | GLubyte(_LineColors[Line] >> 16) | (GLubyte(_LineColors[Line]) << 16);

		for (i = 0; i < Len; ++i) {
			ch = Text[i];
			x1 = x + _Font->m_CharWidth[ch];

			TextObj->m_TextVerts.push_back(Vec2(x, y));
			TextObj->m_TextVerts.push_back(Vec2(x1, y));
			TextObj->m_TextVerts.push_back(Vec2(x, y1));
			TextObj->m_TextVerts.push_back(Vec2(x1, y));
			TextObj->m_TextVerts.push_back(Vec2(x1, y1));
			TextObj->m_TextVerts.push_back(Vec2(x, y1));

			TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU0[ch], _Font->m_CharV0[ch]));
			TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU1[ch], _Font->m_CharV0[ch]));
			TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU0[ch], _Font->m_CharV1[ch]));
			TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU1[ch], _Font->m_CharV0[ch]));
			TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU1[ch], _Font->m_CharV1[ch]));
			TextObj->m_TextUVs.push_back(Vec2(_Font->m_CharU0[ch], _Font->m_CharV1[ch]));

			if (_LineColors != NULL) {
				TextObj->m_Colors.push_back(LineColor);
				TextObj->m_Colors.push_back(LineColor);
				TextObj->m_Colors.push_back(LineColor);
				TextObj->m_Colors.push_back(LineColor);
				TextObj->m_Colors.push_back(LineColor);
				TextObj->m_Colors.push_back(LineColor);
			}

			x = x1;
		}
		if (_BgWidth > 0) {
			TextObj->m_BgVerts.push_back(Vec2(-1, y));
			TextObj->m_BgVerts.push_back(Vec2(_BgWidth + 1, y));
			TextObj->m_BgVerts.push_back(Vec2(-1, y1));
			TextObj->m_BgVerts.push_back(Vec2(_BgWidth + 1, y));
			TextObj->m_BgVerts.push_back(Vec2(_BgWidth + 1, y1));
			TextObj->m_BgVerts.push_back(Vec2(-1, y1));

			if (_LineBgColors != NULL) {
				color32 LineBgColor = (_LineBgColors[Line] & 0xff00ff00) | GLubyte(_LineBgColors[Line] >> 16) | (GLubyte(_LineBgColors[Line]) << 16);
				TextObj->m_BgColors.push_back(LineBgColor);
				TextObj->m_BgColors.push_back(LineBgColor);
				TextObj->m_BgColors.push_back(LineBgColor);
				TextObj->m_BgColors.push_back(LineBgColor);
				TextObj->m_BgColors.push_back(LineBgColor);
				TextObj->m_BgColors.push_back(LineBgColor);
			}
		}
	}
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::DrawText(void *_TextObj, int _X, int _Y, color32 _Color, color32 _BgColor) {
	assert(m_Drawing == true);
	assert(_TextObj != NULL);
	CTextObj *TextObj = static_cast<CTextObj *>(_TextObj);

	if (TextObj->m_TextVerts.size() < 4 && TextObj->m_BgVerts.size() < 4)
		return; // nothing to draw

	GL::_glMatrixMode(GL_MODELVIEW);
	GL::_glLoadIdentity();
	GL::_glTranslatef((GLfloat)_X, (GLfloat)_Y, 0);
	GL::_glEnableClientState(GL_VERTEX_ARRAY);
	if ((_BgColor != 0 || TextObj->m_BgColors.size() == TextObj->m_BgVerts.size()) && TextObj->m_BgVerts.size() >= 4) {
		GL::_glDisable(GL_TEXTURE_2D);
		GL::_glVertexPointer(2, GL_FLOAT, 0, &(TextObj->m_BgVerts[0]));
		if (TextObj->m_BgColors.size() == TextObj->m_BgVerts.size() && _BgColor == 0) {
			GL::_glEnableClientState(GL_COLOR_ARRAY);
			GL::_glColorPointer(4, GL_UNSIGNED_BYTE, 0, &(TextObj->m_BgColors[0]));
		} else {
			GL::_glDisableClientState(GL_COLOR_ARRAY);
			GL::_glColor4ub(GLubyte(_BgColor >> 16), GLubyte(_BgColor >> 8), GLubyte(_BgColor), GLubyte(_BgColor >> 24));
		}
		GL::_glDrawArrays(GL_TRIANGLES, 0, (int)TextObj->m_BgVerts.size());
	}
	GL::_glEnable(GL_TEXTURE_2D);
	GL::_glBindTexture(GL_TEXTURE_2D, m_FontTexID);
	GL::_glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if (TextObj->m_TextVerts.size() >= 4) {
		GL::_glVertexPointer(2, GL_FLOAT, 0, &(TextObj->m_TextVerts[0]));
		GL::_glTexCoordPointer(2, GL_FLOAT, 0, &(TextObj->m_TextUVs[0]));
		if (TextObj->m_Colors.size() == TextObj->m_TextVerts.size() && _Color == 0) {
			GL::_glEnableClientState(GL_COLOR_ARRAY);
			GL::_glColorPointer(4, GL_UNSIGNED_BYTE, 0, &(TextObj->m_Colors[0]));
		} else {
			GL::_glDisableClientState(GL_COLOR_ARRAY);
			GL::_glColor4ub(GLubyte(_Color >> 16), GLubyte(_Color >> 8), GLubyte(_Color), GLubyte(_Color >> 24));
		}

		GL::_glDrawArrays(GL_TRIANGLES, 0, (int)TextObj->m_TextVerts.size());
	}

	GL::_glDisableClientState(GL_VERTEX_ARRAY);
	GL::_glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	GL::_glDisableClientState(GL_COLOR_ARRAY);
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::ChangeViewport(int _X0, int _Y0, int _Width, int _Height, int _OffsetX, int _OffsetY) {
	if (_Width > 0 && _Height > 0) {
		GLint vp[4];
		vp[0] = _X0;
		vp[1] = _Y0;
		vp[2] = _Width - 1;
		vp[3] = _Height - 1;
		GL::_glViewport(vp[0], m_WndHeight - vp[1] - vp[3], vp[2], vp[3]);

		GLint matrixMode = 0;
		GL::_glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
		GL::_glMatrixMode(GL_PROJECTION);
		GL::_glLoadIdentity();
		GL::_glOrtho(_OffsetX, _OffsetX + vp[2], vp[3] - _OffsetY, -_OffsetY, -1, 1);
		GL::_glMatrixMode(matrixMode);
	}
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::RestoreViewport() {
	GL::_glViewport(m_ViewportInit[0], m_ViewportInit[1], m_ViewportInit[2], m_ViewportInit[3]);

	GLint matrixMode = 0;
	GL::_glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	GL::_glMatrixMode(GL_PROJECTION);
	GL::_glLoadMatrixf(m_ProjMatrixInit);
	GL::_glMatrixMode(matrixMode);
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::SetScissor(int _X0, int _Y0, int _Width, int _Height) {
	if (_Width > 0 && _Height > 0) {
		GL::_glScissor(_X0 - 1, m_WndHeight - _Y0 - _Height, _Width - 1, _Height);
		GL::_glEnable(GL_SCISSOR_TEST);
	} else
		GL::_glDisable(GL_SCISSOR_TEST);
}

//  ---------------------------------------------------------------------------

void CTwGraphOpenGL::DrawTriangles(int _NumTriangles, int *_Vertices, color32 *_Colors, Cull _CullMode) {
	assert(m_Drawing == true);

	const GLfloat dx = +0.0f;
	const GLfloat dy = +0.0f;

	GLint prevCullFaceMode, prevFrontFace;
	GL::_glGetIntegerv(GL_CULL_FACE_MODE, &prevCullFaceMode);
	GL::_glGetIntegerv(GL_FRONT_FACE, &prevFrontFace);
	GLboolean prevCullEnable = GL::_glIsEnabled(GL_CULL_FACE);
	GL::_glCullFace(GL_BACK);
	GL::_glEnable(GL_CULL_FACE);
	if (_CullMode == CULL_CW)
		GL::_glFrontFace(GL_CCW);
	else if (_CullMode == CULL_CCW)
		GL::_glFrontFace(GL_CW);
	else
		GL::_glDisable(GL_CULL_FACE);

	GL::_glDisable(GL_TEXTURE_2D);
	GL::_glMatrixMode(GL_MODELVIEW);
	GL::_glLoadIdentity();
	GL::_glBegin(GL_TRIANGLES);
	for (int i = 0; i < 3 * _NumTriangles; ++i) {
		color32 col = _Colors[i];
		GL::_glColor4ub(GLubyte(col >> 16), GLubyte(col >> 8), GLubyte(col), GLubyte(col >> 24));
		GL::_glVertex2f((GLfloat)_Vertices[2 * i + 0] + dx, (GLfloat)_Vertices[2 * i + 1] + dy);
	}
	GL::_glEnd();

	GL::_glCullFace(prevCullFaceMode);
	GL::_glFrontFace(prevFrontFace);
	if (prevCullEnable)
		GL::_glEnable(GL_CULL_FACE);
	else
		GL::_glDisable(GL_CULL_FACE);
}
