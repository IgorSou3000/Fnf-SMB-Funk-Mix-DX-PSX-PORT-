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

	//WORLD2A.XA
	{XA_World2A, XA_LENGTH(13400)}, //XA_Deep_Deep_Voyage
	{XA_World2A, XA_LENGTH(8700)}, //XA_Hop_Hop_Heights
	//WORLD2B.XA
	{XA_World2B, XA_LENGTH(12800)}, //XA_Koopa_Armada

	//FREE1A.XA
	{XA_Free1A, XA_LENGTH(8600)}, //XA_2_Player_Game
	{XA_Free1A, XA_LENGTH(13100)}, //XA_Destruction_Dance
	//FREE1B.XA
	{XA_Free1B, XA_LENGTH(10000)}, //XA_Portal_Power

	//FREE2A.XA
	{XA_Free2A, XA_LENGTH(13500)}, //XA_Bullet_Time
	{XA_Free2A, XA_LENGTH(13400)}, //XA_Boo_Blitz
	//FREE2B.XA
	{XA_Free2B, XA_LENGTH(13200)}, //XA_Cross_Console_Clash

	//SECRETA.XA
	{XA_SecretA, XA_LENGTH(10700)}, //XA_Wrong_Warp
	{XA_SecretA, XA_LENGTH(10500)}, //XA_First_Level
	//SECRETB.XA
	{XA_SecretB, XA_LENGTH(10500)}, //XA_Green_Screen
	{XA_SecretB, XA_LENGTH(6400)}, //XA_Balls
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //XA_Menu
	"\\MUSIC\\WORLD1A.XA;1", //XA_World1A
	"\\MUSIC\\WORLD1B.XA;1", //XA_World1B
	"\\MUSIC\\WORLD2A.XA;1", //XA_World2A
	"\\MUSIC\\WORLD2B.XA;1", //XA_World2B
	"\\MUSIC\\FREE1A.XA;1", //XA_Free1A
	"\\MUSIC\\FREE1B.XA;1", //XA_Free1B
	"\\MUSIC\\FREE2A.XA;1", //XA_Free2A
	"\\MUSIC\\FREE2B.XA;1", //XA_Free2B
	"\\MUSIC\\SECRETA.XA;1", //XA_SecretA
	"\\MUSIC\\SECRETB.XA;1", //XA_SecretB
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

	//WORLD2A.XA
	{"deep_deep_voyage", true}, //XA_Deep_Deep_Voyage
	{"hop_hop_heights", true},   //XA_Hop_Hop_Heights
	//WORLD2B.XA
	{"koopa_armada", true}, //XA_Koopa_Armada

	//FREE1A.XA
	{"2_player_game", true}, //XA_2_Player_Game
	{"destruction_dance", true},   //Destruction_Dance
	//FREE1B.XA
	{"portal_power", true}, //XA_Portal_Power

	//FREE2A.XA
	{"bullet_time", true}, //XA_Bullet_Time
	{"boo-blitz", true},   //XA_Boo_Blitz
	//FREE2B.XA
	{"cross_console_clash", true}, //XA_Cross_Console_Clash

	//SECRETA.XA
	{"wrong_warp", true}, //XA_Wrong_Warp
	{"first_level", true}, //XA_First_Level
	//SECRETB.XA
	{"green_screen", true}, //XA_Green_Screen
	{"balls", false}, //XA_Balls
	
	{NULL, false}
};
