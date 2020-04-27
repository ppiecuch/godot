## Simple 2d water surfaces


### Shader

* Perturbate coordinates of background mapping with the components X,Y of normals... simulate refraction

```
m_newuvmap[x][y][0] = m_uvmap[x][y][0] + 0.05f * m_snormaln[x][y][0]
m_newuvmap[x][y][1] = m_uvmap[x][y][1] + 0.05f * m_snormaln[x][y][1]
```

* Trick: xy projection of normals ->  assume reflection in direction of the normals; looks ok for non-plane surfaces

```
m_envmap[x][y][0] = 0.5f + m_snormaln[x][y][0] * 0.45f
m_envmap[x][y][1] = 0.5f + m_snormaln[x][y][1] * 0.45f
```

* Shaders conversion:

```
attribute vec3 iPos;
attribute vec3 iNormal;
attribute vec2 iTex0;

uniform mat4 mvp;
uniform bias      = {0.5, 0.5, 0.5, 1}
uniform scale     = {0.45, 0.45, 0.45, 0}
uniform perturb   = {0.05, 0.05, 0.05, 0}

varying vec4 oPos;
varying vec2 oTex0;
varying vec2 oTex1;

oPos.x = dot(iPos, mvp[0]);
oPos.y = dot(iPos, mvp[1]);
oPos.z = dot(iPos, mvp[2]);
oPos.w = dot(iPos, mvp[3]);
// normalize normals
vec3 tmp;
tmp.w = dot(iNormal, iNormal);
tmp.w = 1.0 / sqrt(tmp.w)
tmp.xyz = tmp.w * iNormal;
oTex0 = tmp * pertub + iTex0;
oTex1 = tmp * scale + bias;
```

ARB source code:

```
// The ARB Vertex Program with Auto-normalize String
const char my_Vertex_Program_Normalize[] = "
   !!ARBvp1.0\
	ATTRIB iPos		= vertex.position;\n\
	ATTRIB iNormal  = vertex.normal;\n\
	ATTRIB iTex0	= vertex.texcoord[0];\n\
	PARAM mvp[4]	= { state.matrix.mvp };\n\
	PARAM bias      = {0.5, 0.5, 0.5, 1};\n\
	PARAM scale     = {0.45, 0.45, 0.45, 0};\n\
	PARAM perturb   = {0.05, 0.05, 0.05, 0};\n\
	OUTPUT oPos		= result.position;\n\
	OUTPUT oTex0	= result.texcoord[0];\n\
	OUTPUT oTex1	= result.texcoord[1];\n\
	TEMP tmp;\n\
	DP4 oPos.x, mvp[0], iPos;\n\
	DP4 oPos.y, mvp[1], iPos;\n\
	DP4 oPos.z, mvp[2], iPos;\n\
	DP4 oPos.w, mvp[3], iPos;\n\
	# normalize normals\n\
	DP3 tmp.w, iNormal, iNormal;\n\
	RSQ tmp.w, tmp.w;\n\
	MUL tmp.xyz, tmp.w, iNormal;\n\
	MAD oTex0, tmp, perturb, iTex0;\n\
	MAD oTex1, tmp, scale, bias;\n\
	END
";

// The ARB Vertex Program w/o Normalize String
const char my_Vertex_Program[] = "
	!!ARBvp1.0\
	ATTRIB iPos		= vertex.position;\n\
	ATTRIB iNormal  = vertex.normal;\n\
	ATTRIB iTex0	= vertex.texcoord[0];\n\
	PARAM mvp[4]	= { state.matrix.mvp };\n\
	PARAM bias      = {0.5, 0.5, 0.5, 1};\n\
	PARAM scale     = {0.45, 0.45, 0.45, 0};\n\
	PARAM perturb   = {0.05, 0.05, 0.05, 0};\n\
	OUTPUT oPos		= result.position;\n\
	OUTPUT oTex0	= result.texcoord[0];\n\
	OUTPUT oTex1	= result.texcoord[1];\n\
	TEMP tmp;\n\
	DP4 oPos.x, mvp[0], iPos;\n\
	DP4 oPos.y, mvp[1], iPos;\n\
	DP4 oPos.z, mvp[2], iPos;\n\
	DP4 oPos.w, mvp[3], iPos;\n\
	MAD oTex0, iNormal, perturb, iTex0;\n\
	MAD oTex1, iNormal, scale, bias;\n\
	END
";
```
