#ifdef WITH_FBT

#ifndef _KX_BGEDNA
#define _KX_BGEDNA
/*FINAL TODO: ALL THE DATA MUST BE 4BIT ALIGNED, SO CALCULATE ALL THE SIZE OF EACH STRUCT AND CORRECT IF WROND WITH pad0, pad1, ..., padN chars or shorts*/
/*NOTE: SG_QList and SG_DList are used like our class List. So there is no reason to create a new
class, just use List.
/*makefbt doesn't like public methods and typedef inside classes.
So in classes first define all private and protected methods and attribute,
then use "pub_methods:" to define public methods, and lately use "public:" to
define all the public attributes.
Remeber to bring typedef outside of classes.*/
#define pub_methods public 

namespace Bgedna {
/** \addtogroup Bgedna
*  @{
*/

/*data list related code*/

class List
{
pub_methods:
	List() {next = 0;};
public:
	List	*next;
	List	*prev;
};

class PointerList : public List
{
public:
	void* data;
};


class fbtList
{
pub_methods:

	fbtList() : first(0), last(0) {}
	fbtList(unsigned short ID_code) : ID(ID_code), first(0), last(0) {}
	~fbtList() { clear(); }

	void clear(void) { first = last = 0; }

	void push_back(void* v)
	{
		List* datalist = ((List*)v);
		if (!datalist)
			return;

		datalist->prev = last;
		if (last)
			last->next = datalist;

		if (!first)
			first = datalist;

		last = datalist;
	}

	void remove(List* link)
	{
		if (!link)
			return;
		if (link->next)
			link->next->prev = link->prev;
		if (link->prev)
			link->prev->next = link->next;
		if (last == link)
			last = link->prev;
		if (first == link)
			first = link->next;
	}

	void setIDCode(unsigned int IDcode) { ID = IDcode; }
	unsigned int getIDCode() {return ID;};

public:
	List	*first;
	List	*last;
	unsigned short ID;
};

class CValueStruct : public List
{

};

class KX_GameObjectStruct :CValueStruct
{
	
};

class KX_FontObjectStruct : KX_GameObjectStruct
{

};

class KX_CameraStruct : KX_GameObjectStruct
{

};

class KX_LightObjectStruct : KX_GameObjectStruct
{

};

class KX_WorldInfoStruct : public List
{
public:
	float		back_color[3];
	float		ambient_color[3];
	bool		has_mist;
	float		mist_color[3];
	float		mist_distance;
	float		mist_start;
	bool		has_world;
};

class RAS_FrameSettingsStruct : public List
{
public:
	int						m_frame_type;//it is an enum
	float					bar[3];
	unsigned int			m_design_aspect_width;
	unsigned int			m_design_aspect_height;
};

class RAS_IPolyMaterialStruct : public List
{
public:
	char					m_texturename[128];
	char					m_materialname[128];
	int						m_tile;
	int						m_tilexrep,m_tileyrep;
	int						m_drawingmode;
	int						m_transp;
	bool					m_alpha;
	bool					m_zsort;
	int						m_materialindex;

	float					m_diffuse[3];
	float					m_shininess;
	float					m_specular[3];
	float					m_specularity;
};

class RAS_TexVertStruct : public List
{
public:
	float			m_localxyz[3];	// 3*4 =	12
	float			m_uv1[2];		// 2*4 =	8
	float			m_uv2[2];		// 2*4 =	8
	unsigned int	m_rgba;			//			4
	float			m_tangent[4];   // 4*4 =	16
	float			m_normal[3];	// 3*4 =	12
	short			m_flag;			//		    2
	short			m_softBodyIndex;//			2
	unsigned int	m_unit;			//			4
	unsigned int	m_origindex;	//			4
	//									total = 72
};

/*yes, it is stupid, but since i use fbtList to store lists types,
i thought was better to create this class that finding another method for
saving a vector of ushort..*/
class ushortList : public List
{
public:
	unsigned short value;
};

class RAS_DisplayArrayStruct : public List
{
public:
	int			m_type;//enum
	int			m_users;
	fbtList		m_vertex;
	fbtList		m_index;
};

class RAS_MeshObjectStruct : public List
{

};

class RAS_DeformerStruct : public List
{

};

class DerivedMeshStruct : public List
{

};

/*is it actually usefull? This structs should only store what is needed after the conversion,
not all the element to do a dump of the memory on disk.. */
class KX_ListSlotStruct : public List
{

};

class RAS_MaterialBucketStruct : public List
{
public:
	bool						isSorted;
	bool						isAlpha;
	RAS_IPolyMaterialStruct		*material;
	fbtList						mesh_slot;
	fbtList						act_mesh_slot;

};

class RAS_MeshSlotStruct : public List
{
public:
	//  indices into display arrays
	int							m_startarray;
	int							m_endarray;
	int							m_startindex;
	int							m_endindex;
	int							m_startvertex;
	int							m_endvertex;
	fbtList						m_displayArrays;

	// for construction only
	RAS_DisplayArrayStruct		*m_currentArray;

	// for rendering
	RAS_MaterialBucketStruct	*m_bucket;
	RAS_MeshObjectStruct		*m_mesh;
	void						*m_clientObj;
	RAS_DeformerStruct			*m_pDeformer;
	DerivedMeshStruct			*m_pDerivedMesh;
	double						*m_OpenGLMatrix;
	// visibility
	bool						m_bVisible;
	bool						m_bCulled;
	// object color
	bool						m_bObjectColor;
	float						m_RGBAcolor[4];
	// display lists
	KX_ListSlotStruct			*m_DisplayList;
	bool						m_bDisplayList;
	// joined mesh slots
	RAS_MeshSlotStruct			*m_joinSlot;
	/**
	 * Access with [row index][column index]
	 */
	float						m_joinInvTransform[4][4];
	/*note: FBT supports at max 2 dimension matrices, DON'T exceed this limit!!*/
	fbtList						m_joinedSlots;
};

class RAS_BucketManagerStruct : public List
{
public:
	fbtList					solid_bucket_material_list;
	fbtList					alpha_bucket_material_list;
};

class RAS_RectStruct : public List
{
public:
	int m_x1, m_y1;
	int m_x2, m_y2;
};

class SCA_TimeEventManagerStruct : public List
{

};

/*fbt works with public attributes, so since all the attributes in KX_Scene are protected, i create a struct that will contains
all the attributes retrieved with the KX_Scene's pubblic methods.*/
class KX_SceneStruct : public List
{
pub_methods:
	KX_SceneStruct(){ List();};
public:
	//put here all the variables that will contain KX_Scene's attributes that must be saved
	KX_CameraStruct				*active_camera;
	RAS_BucketManagerStruct		*bucket_manager;
	fbtList						cameras;
	unsigned int				camera_design_height;
	unsigned int				camera_design_width;
	bool						dbvt_culling;
	int							dbvt_occlusion_res;
	fbtList						fonts;
	RAS_FrameSettingsStruct		*framing_type;
	fbtList						inactive_list;
	fbtList						light_list;
	char						name[128];
	fbtList						object_list;
	KX_CameraStruct				*pcamera;

	double						suspended_delta;
	double						suspended_time;
	fbtList						temp_obj_list;

	RAS_RectStruct				*scene_viewport;
	SCA_TimeEventManagerStruct	*time_event_manager;
	KX_WorldInfoStruct			*world_info;

};

class FileGlobal
{
public:
    char				subvstr[4];
    short				subversion;
    int					revision;
    char				filename[240];
	KX_SceneStruct		*current_scene;
};

/** @}*/
}
#endif//_KX_BGEDNA
#endif