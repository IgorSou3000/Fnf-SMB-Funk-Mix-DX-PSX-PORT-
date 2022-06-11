#define XA_LENGTH(x) (((u64)(x) * 75) / 100 * IO_SECT_SIZE) //Centiseconds to sectors in bytes (w)

typedef struct
{
	XA_File file;
	u32 length;
} XA_TrackDef;

static const XA_TrackDef xa_tracks[] = {
	//MENU.XA
	{XA_Menu, XA_LENGTH(11295)}, //XA_GettinFreaky
	{XA_Menu, XA_LENGTH(3840)},  //XA_GameOver
	//WORLD1A.XA
	{XA_World1A, XA_LENGTH(9100)}, //XA_Mushroom_Plains
	{XA_World1A, XA_LENGTH(10400)}, //XA_Bricks_And_Lifts
	//WORLD1B.XA
	{XA_World1B, XA_LENGTH(16100)}, //XA_Lethal_Lava_Lair
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //XA_Menu
	"\\MUSIC\\WORLD1A.XA;1", //XA_World1A
	"\\MUSIC\\WORLD1B.XA;1", //XA_World1B
	NULL,
};

typedef struct
{
	const char *name;
	boolean vocal;
} XA_Mp3;

static const XA_Mp3 xa_mp3s[] = {
	//MENU.XA
	{"freaky", false},   //XA_GettinFreaky
	{"gameover", false}, //XA_GameOver
	//WORLD1A.XA
	{"mushroom_plains", true}, //XA_Mushroom_Plains
	{"bricks_and_lifts", true},   //XA_Bricks_And_Lifts
	//WORLD1B.XA
	{"lethal_lava_lair", true}, //XA_Lethal_Lava_Lair
	
	{NULL, false}
};
