--
-- Copyright (c) 2013-2021 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

--
-- add large parameters and output tables
--

-- 
-- modelOne large types
--
-- years  type id 105, enum ids: 0, 1, 2, 3, 4, ...., 200  = 201 item
-- period type id 106, enum ids: 1, 3, 5, 7, 9, ...., 401  = 201 item
-- low    type id 107, enum ids: 1, 3, 5, 7, 9, ...., 19   =  10 items
-- middle type id 108, enum ids: 21, 23, 25,    ...., 79   =  30 items
--
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (105, 'years', '_20128171604590125', 2, 1024);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (106, 'period', '_20128171604590126', 2, 2048);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (107, 'low', '_20128171604590127', 2, 4096);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (108, 'middle', '_20128171604590128', 2, 8192);

INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 105, 105);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 106, 106);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 107, 107);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 108, 108);

INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (105, 0, 'Experience', 'Years of experience');
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (105, 1, 'Vivre', 'Des années d''expérience');
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (106, 0, 'Period', 'Last job duration');
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (106, 1, 'Période', 'Durée du dernier emploi');
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (107, 0, 'Low Period', NULL);
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (108, 0, 'Middle Period', NULL);

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 0,   '< 1');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 1,   '[1, 2)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 2,   '[2, 3)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 3,   '[3, 4)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 4,   '[4, 5)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 5,   '[5, 6)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 6,   '[6, 7)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 7,   '[7, 8)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 8,   '[8, 9)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 9,   '[9, 10)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 10,  '[10, 11)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 11,  '[11, 12)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 12,  '[12, 13)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 13,  '[13, 14)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 14,  '[14, 15)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 15,  '[15, 16)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 16,  '[16, 17)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 17,  '[17, 18)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 18,  '[18, 19)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 19,  '[19, 20)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 20,  '[20, 21)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 21,  '[21, 22)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 22,  '[22, 23)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 23,  '[23, 24)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 24,  '[24, 25)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 25,  '[25, 26)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 26,  '[26, 27)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 27,  '[27, 28)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 28,  '[28, 29)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 29,  '[29, 30)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 30,  '[30, 31)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 31,  '[31, 32)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 32,  '[32, 33)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 33,  '[33, 34)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 34,  '[34, 35)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 35,  '[35, 36)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 36,  '[36, 37)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 37,  '[37, 38)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 38,  '[38, 39)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 39,  '[39, 40)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 40,  '[40, 41)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 41,  '[41, 42)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 42,  '[42, 43)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 43,  '[43, 44)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 44,  '[44, 45)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 45,  '[45, 46)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 46,  '[46, 47)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 47,  '[47, 48)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 48,  '[48, 49)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 49,  '[49, 50)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 50,  '[50, 51)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 51,  '[51, 52)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 52,  '[52, 53)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 53,  '[53, 54)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 54,  '[54, 55)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 55,  '[55, 56)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 56,  '[56, 57)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 57,  '[57, 58)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 58,  '[58, 59)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 59,  '[59, 60)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 60,  '[60, 61)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 61,  '[61, 62)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 62,  '[62, 63)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 63,  '[63, 64)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 64,  '[64, 65)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 65,  '[65, 66)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 66,  '[66, 67)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 67,  '[67, 68)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 68,  '[68, 69)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 69,  '[69, 70)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 70,  '[70, 71)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 71,  '[71, 72)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 72,  '[72, 73)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 73,  '[73, 74)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 74,  '[74, 75)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 75,  '[75, 76)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 76,  '[76, 77)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 77,  '[77, 78)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 78,  '[78, 79)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 79,  '[79, 80)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 80,  '[80, 81)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 81,  '[81, 82)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 82,  '[82, 83)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 83,  '[83, 84)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 84,  '[84, 85)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 85,  '[85, 86)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 86,  '[86, 87)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 87,  '[87, 88)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 88,  '[88, 89)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 89,  '[89, 90)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 90,  '[90, 91)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 91,  '[91, 92)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 92,  '[92, 93)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 93,  '[93, 94)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 94,  '[94, 95)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 95,  '[95, 96)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 96,  '[96, 97)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 97,  '[97, 98)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 98,  '[98, 99)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 99,  '[99, 100)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 100,  '[100, 101)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 101,  '[101, 102)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 102,  '[102, 103)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 103,  '[103, 104)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 104,  '[104, 105)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 105,  '[105, 106)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 106,  '[106, 107)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 107,  '[107, 108)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 108,  '[108, 109)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 109,  '[109, 110)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 110,  '[110, 111)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 111,  '[111, 112)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 112,  '[112, 113)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 113,  '[113, 114)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 114,  '[114, 115)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 115,  '[115, 116)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 116,  '[116, 117)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 117,  '[117, 118)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 118,  '[118, 119)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 119,  '[119, 120)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 120,  '[120, 121)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 121,  '[121, 122)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 122,  '[122, 123)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 123,  '[123, 124)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 124,  '[124, 125)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 125,  '[125, 126)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 126,  '[126, 127)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 127,  '[127, 128)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 128,  '[128, 129)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 129,  '[129, 130)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 130,  '[130, 131)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 131,  '[131, 132)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 132,  '[132, 133)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 133,  '[133, 134)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 134,  '[134, 135)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 135,  '[135, 136)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 136,  '[136, 137)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 137,  '[137, 138)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 138,  '[138, 139)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 139,  '[139, 140)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 140,  '[140, 141)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 141,  '[141, 142)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 142,  '[142, 143)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 143,  '[143, 144)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 144,  '[144, 145)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 145,  '[145, 146)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 146,  '[146, 147)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 147,  '[147, 148)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 148,  '[148, 149)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 149,  '[149, 150)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 150,  '[150, 151)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 151,  '[151, 152)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 152,  '[152, 153)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 153,  '[153, 154)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 154,  '[154, 155)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 155,  '[155, 156)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 156,  '[156, 157)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 157,  '[157, 158)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 158,  '[158, 159)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 159,  '[159, 160)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 160,  '[160, 161)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 161,  '[161, 162)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 162,  '[162, 163)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 163,  '[163, 164)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 164,  '[164, 165)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 165,  '[165, 166)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 166,  '[166, 167)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 167,  '[167, 168)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 168,  '[168, 169)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 169,  '[169, 170)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 170,  '[170, 171)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 171,  '[171, 172)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 172,  '[172, 173)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 173,  '[173, 174)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 174,  '[174, 175)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 175,  '[175, 176)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 176,  '[176, 177)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 177,  '[177, 178)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 178,  '[178, 179)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 179,  '[179, 180)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 180,  '[180, 181)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 181,  '[181, 182)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 182,  '[182, 183)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 183,  '[183, 184)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 184,  '[184, 185)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 185,  '[185, 186)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 186,  '[186, 187)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 187,  '[187, 188)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 188,  '[188, 189)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 189,  '[189, 190)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 190,  '[190, 191)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 191,  '[191, 192)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 192,  '[192, 193)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 193,  '[193, 194)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 194,  '[194, 195)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 195,  '[195, 196)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 196,  '[196, 197)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 197,  '[197, 198)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 198,  '[198, 199)');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 199,  '[199, 200)');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (105, 200, '200+');

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name)
SELECT 106, 2 * enum_id + 1, enum_name FROM type_enum_lst WHERE type_hid = 105;

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name)
SELECT 107, enum_id, enum_name FROM type_enum_lst WHERE type_hid = 106 AND enum_id < 20;

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name)
SELECT 108, enum_id, enum_name FROM type_enum_lst WHERE type_hid = 106 AND enum_id BETWEEN 20 AND 80;

INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) SELECT type_hid, enum_id, 0, enum_name, NULL FROM type_enum_lst WHERE type_hid IN (105, 106, 107, 108);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) SELECT type_hid, enum_id, 1, enum_name, NULL FROM type_enum_lst WHERE type_hid IN (105, 106, 107, 108);

-- 
-- modelOne input parameters
--
--   salaryByYears, id 108 rank 4: [age, sex, salary, years]         =   4824 rows
--   salaryByPeriod id 109 rank 5: [age, sex, salary, years, period] = 969624 rows
--   salaryByLow    id 110 rank 5: [age, sex, salary, years, low]    =  48240 rows
--   salaryByMiddle id 111 rank 5: [age, sex, salary, years, middle] = 144720 rows
--

-- 108: salaryByYears rank 4 [age, sex, salary, years]

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
VALUES
  (108, 'salaryByYears', '_20128171604590138', 4, 14, 0, 0, 'salaryByYears_p_2012818', 'salaryByYears_w_2012818', '_i0128171604590138');

INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 7, 108, 0);

INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (108, 0, 'Salary by Years', 'Salary By Years notes');

INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (108, 0, 'dim0', 101);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (108, 1, 'dim1', 102);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (108, 2, 'dim2', 103);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (108, 3, 'dim3', 105);

INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (108, 0, 0, 'Age', 'Age Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (108, 0, 1, 'Âge', 'Âge Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (108, 1, 0, 'Sex', 'Sex Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (108, 1, 1, 'Sexe', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (108, 2, 0, 'Full or Part', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (108, 2, 1, 'Plein ou partiel', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (108, 3, 0, 'Experience', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (108, 3, 1, 'Vivre', NULL);

-- 109: salaryByPeriod rank 5 [age, sex, salary, years, period]

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
VALUES
  (109, 'salaryByPeriod', '_20128171604590139', 5, 14, 0, 0, 'salaryByPeriod_p_2012819', 'salaryByPeriod_w_2012819', '_i0128171604590139');

INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 8, 109, 0);

INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (109, 0, 'Salary by Period', 'Salary by Period notes');

INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (109, 0, 'dim0', 101);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (109, 1, 'dim1', 102);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (109, 2, 'dim2', 103);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (109, 3, 'dim3', 105);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (109, 4, 'dim4', 106);

INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (109, 0, 0, 'Age', 'Age Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (109, 0, 1, 'Âge', 'Âge Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (109, 1, 0, 'Sex', 'Sex Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (109, 1, 1, 'Sexe', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (109, 2, 0, 'Full or Part', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (109, 2, 1, 'Plein ou partiel', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (109, 3, 0, 'Experience', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (109, 3, 1, 'Vivre', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (109, 4, 0, 'Period', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (109, 4, 1, 'Période', NULL);

-- 110: salaryByLow rank 5 [age, sex, salary, years, low]

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
VALUES
  (110, 'salaryByLow', '_20128171604590_10', 5, 14, 0, 0, 'salaryByLow_p_201281_10', 'salaryByLow_w_201281_10', '_i0128171604590_10');

INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 9, 110, 1);

INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (110, 0, 'Salary by Low', 'Salary by Low Period notes');

INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (110, 0, 'dim0', 101);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (110, 1, 'dim1', 102);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (110, 2, 'dim2', 103);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (110, 3, 'dim3', 105);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (110, 4, 'dim4', 107);

INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (110, 0, 0, 'Age', 'Age Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (110, 0, 1, 'Âge', 'Âge Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (110, 1, 0, 'Sex', 'Sex Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (110, 1, 1, 'Sexe', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (110, 2, 0, 'Full or Part', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (110, 2, 1, 'Plein ou partiel', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (110, 3, 0, 'Experience', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (110, 3, 1, 'Vivre', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (110, 4, 0, 'Low Period', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (110, 4, 1, 'Période Basse', NULL);

-- 111: salaryByMiddle rank 5 [age, sex, salary, years, middle]

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
VALUES
  (111, 'salaryByMiddle', '_20128171604590_11', 5, 14, 0, 0, 'salaryByMiddle_p_201281_11', 'salaryByMiddle_w_201281_11', '_i0128171604590_11');

INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1, 10, 111, 1);

INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (111, 0, 'Salary by Middle', 'Salary by Middle Period notes');

INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (111, 0, 'dim0', 101);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (111, 1, 'dim1', 102);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (111, 2, 'dim2', 103);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (111, 3, 'dim3', 105);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (111, 4, 'dim4', 108);

INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (111, 0, 0, 'Age', 'Age Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (111, 0, 1, 'Âge', 'Âge Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (111, 1, 0, 'Sex', 'Sex Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (111, 1, 1, 'Sexe', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (111, 2, 0, 'Full or Part', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (111, 2, 1, 'Plein ou partiel', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (111, 3, 0, 'Experience', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (111, 3, 1, 'Vivre', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (111, 4, 0, 'Middle Period', NULL);
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (111, 4, 1, 'Période Moyenne', NULL);

-- 
-- modelOne output tables
--
-- incomeByYear id 105 rank 4: [age, sex, salary, years] = 4824 rows
--    acumulators: 2 native + 1 derived =  9648 rows
--    expressions: 4                    = 19296 rows
--
-- enum ids for incomeByYear.dim0: 10,  20,  30, 40
-- enum ids for incomeByYear.dim1: 0,   1
-- enum ids for incomeByYear.dim2: 100, 200, 300
-- enum ids for incomeByYear.dim3: 0, 1, 2, 3, 4, ...., 200  = 201 item
--
-- incomeByLow id 106 rank 5: [age, sex, salary, years, low] = 48240 rows
--    acumulators: 2 native + 1 derived =  96480 rows
--    expressions: 4                    = 192960 rows
--
-- enum ids for incomeByLow.dim0: 10,  20,  30, 40
-- enum ids for incomeByLow.dim1: 0,   1
-- enum ids for incomeByLow.dim2: 100, 200, 300
-- enum ids for incomeByLow.dim3: 0, 1, 2, 3, 4, ...., 200  = 201 item
-- enum ids for incomeByLow.dim4: 1, 3, 5, 7, 9, ...., 19   = 10 items
--
-- incomeByMiddle id 107 rank 5: [age, sex, salary, years, middle] = 144720 rows
--    acumulators: 2 native + 1 derived = 289440 rows
--    expressions: 4                    = 578880 rows
--
-- enum ids for incomeByMiddle.dim0: 10,  20,  30, 40
-- enum ids for incomeByMiddle.dim1: 0,   1
-- enum ids for incomeByMiddle.dim2: 100, 200, 300
-- enum ids for incomeByMiddle.dim3: 0, 1, 2, 3, 4, ...., 200  = 201 item
-- enum ids for incomeByMiddle.dim4: 21, 23, 25, ...., 79      = 30 items
--
-- incomeByPeriod id 108 rank 5: [age, sex, salary, years, period] = 969624 rows
--    acumulators: 2 native + 1 derived = 1939248 rows
--    expressions: 4                    = 3878496 rows
--
-- enum ids for incomeByPeriod.dim0: 10,  20,  30, 40
-- enum ids for incomeByPeriod.dim1: 0,   1
-- enum ids for incomeByPeriod.dim2: 100, 200, 300
-- enum ids for incomeByPeriod.dim3: 0, 1, 2, 3, 4, ...., 200  = 201 item
-- enum ids for incomeByPeriod.dim4: 1, 3, 5, 7, 9, ...., 401  = 201 item
--

--
-- incomeByYear rank 4 [age, sex, salary, years]
--
INSERT INTO table_dic 
  (table_hid, table_name, table_digest, table_rank, is_sparse, db_expr_table, db_acc_table, db_acc_all_view, import_digest) 
VALUES 
  (105, 'incomeByYear', '_20128171604590105', 4, 0, 'incomeByYear_v_2012105', 'incomeByYear_a_2012105', 'incomeByYear_d_2012105', '_i20128171604590105');

INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos, is_hidden) VALUES (1, 4, 105, 0, 0, 1);

INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (105, 0, 'Income by Years', 'Income by Age, Sex, Salary, Years notes', 'Income Measure', 'Income by Years Measure notes');
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (105, 1, 'Revenu par années', 'Revenu par âge, sexe, salaire, années notes', 'Mesure du Revenu', 'Mesure du Revenu par âge, sexe, salaire, années notes');

-- total enum disabled for all dimensions
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (105, 0, 'dim0', 101, 0, 4);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (105, 1, 'dim1', 102, 0, 2);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (105, 2, 'dim2', 103, 0, 3);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (105, 3, 'dim3', 105, 0, 201);

INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (105, 0, 0, 'Age', 'Age Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (105, 0, 1, 'Âge', 'Âge Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (105, 1, 0, 'Sex', 'Sex Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (105, 1, 1, 'Sexe', NULL);
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (105, 2, 0, 'Salary', 'Salary Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (105, 2, 1, 'Salaire', 'Salaire Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (105, 3, 0, 'Years', 'Years of experience Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (105, 3, 1, 'Années', 'Des années d''expérience Dim notes');

INSERT INTO
  table_acc (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql)
VALUES
  (
  105, 0, 'acc0', 0, 'raw_value()',
  'SELECT run_id, sub_id, dim0, dim1, dim2, dim3, acc_value FROM incomeByYear_a_2012105 WHERE acc_id = 0'
  );

INSERT INTO table_acc 
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
VALUES 
  (
  105, 1, 'acc1', 0, 'acc1', 
  'SELECT run_id, sub_id, dim0, dim1, dim2, dim3, acc_value FROM incomeByYear_a_2012105 WHERE acc_id = 1'
  );

INSERT INTO table_acc 
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
VALUES 
  (
  105, 2, 'expr2', 1, 'acc0 - acc1', 
  '(A.acc_value - A1.acc_value)'
  );

INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (105, 0, 0, 'Income', 'Income notes');
INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (105, 1, 0, 'Income adjusted', 'Income adjusted notes');

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (105, 0, 'expr0', 2, 'OM_AVG(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, AVG(M1.acc_value) AS expr0 FROM incomeByYear_a_2012105 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (105, 1, 'expr1', -1, 'OM_AVG(acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, AVG(M1.acc_value) AS expr1 FROM incomeByYear_a_2012105 M1 WHERE M1.acc_id = 1 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (105, 2, 'expr2', 2, 'OM_AVG(acc0 * acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, AVG(M1.acc_value * A1.acc1) AS expr2 FROM incomeByYear_a_2012105 M1 INNER JOIN (SELECT run_id, dim0, dim1, dim2, dim3, sub_id, acc_value AS acc1 FROM incomeByYear_a_2012105 WHERE acc_id = 1) A1 ON (A1.run_id = M1.run_id AND A1.dim0 = M1.dim0 AND A1.dim1 = M1.dim1 AND A1.dim2 = M1.dim2 AND A1.dim3 = M1.dim3 AND A1.sub_id = M1.sub_id) WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (105, 3, 'expr3', 2, 'OM_SUM(acc0 + acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, SUM(M1.acc_value + A1.acc1) AS expr3 FROM incomeByYear_a_2012105 M1 INNER JOIN (SELECT run_id, dim0, dim1, dim2, dim3, sub_id, acc_value AS acc1 FROM incomeByYear_a_2012105 WHERE acc_id = 1) A1 ON (A1.run_id = M1.run_id AND A1.dim0 = M1.dim0 AND A1.dim1 = M1.dim1 AND A1.dim2 = M1.dim2 AND A1.dim3 = M1.dim3 AND A1.sub_id = M1.sub_id) WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3'
  );

INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (105, 0, 0, 'Average acc0', 'Average on acc0 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (105, 1, 0, 'Average acc1', 'Average on acc1 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (105, 2, 0, 'Average acc0 * acc1', 'Average on acc0 * acc1 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (105, 3, 0, 'Sum acc0 + acc1', 'Sum of acc0 + acc1 notes');

--
-- incomeByLow rank 5 [age, sex, salary, years, low] = 48240 rows
--
INSERT INTO table_dic 
  (table_hid, table_name, table_digest, table_rank, is_sparse, db_expr_table, db_acc_table, db_acc_all_view, import_digest) 
VALUES 
  (106, 'incomeByLow', '_20128171604590106', 5, 0, 'incomeByLow_v_2012106', 'incomeByLow_a_2012106', 'incomeByLow_d_2012106', '_i20128171604590106');

INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos, is_hidden) VALUES (1, 5, 106, 0, 0, 0);

INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (106, 0, 'Income by Low Period', 'Income by age, sex, salary, years, low period notes', 'Income Measure', 'Income by Low Period Measure notes');
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (106, 1, 'Revenu par période basse', 'Revenu par âge, sexe, salaire, années, période basse notes', 'Revenu par mesure', 'Notes sur la mesure du revenu par période basse');

-- total enum disabled for all dimensions
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (106, 0, 'dim0', 101, 0, 4);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (106, 1, 'dim1', 102, 0, 2);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (106, 2, 'dim2', 103, 0, 3);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (106, 3, 'dim3', 105, 0, 201);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (106, 4, 'dim4', 107, 0, 10);

INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (106, 0, 0, 'Age', 'Age Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (106, 0, 1, 'Âge', 'Âge Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (106, 1, 0, 'Sex', 'Sex Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (106, 1, 1, 'Sexe', NULL);
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (106, 2, 0, 'Salary', 'Salary Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (106, 2, 1, 'Salaire', 'Salaire Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (106, 3, 0, 'Years', 'Years of experience Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (106, 3, 1, 'Années', 'Des années d''expérience Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (106, 4, 0, 'Low Period', 'Low Period Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (106, 4, 1, 'Période basse', 'Période basse Dim notes');

INSERT INTO table_acc
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql)
VALUES
  (
  106, 0, 'acc0', 0, 'raw_value()',
  'SELECT run_id, sub_id, dim0, dim1, dim2, dim3, dim4, acc_value FROM incomeByLow_a_2012106 WHERE acc_id = 0'
  );

INSERT INTO table_acc 
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
VALUES 
  (
  106, 1, 'acc1', 0, 'acc1', 
  'SELECT run_id, sub_id, dim0, dim1, dim2, dim3, dim4, acc_value FROM incomeByLow_a_2012106 WHERE acc_id = 1'
  );

INSERT INTO table_acc 
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
VALUES 
  (
  106, 2, 'expr2', 1, 'acc0 - acc1', 
  '(A.acc_value - A1.acc_value)'
  );

INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (106, 0, 0, 'Income', 'Income notes');
INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (106, 1, 0, 'Income adjusted', 'Income adjusted notes');

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (106, 0, 'expr0', 2, 'OM_AVG(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, AVG(M1.acc_value) AS expr0 FROM incomeByLow_a_2012106 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (106, 1, 'expr1', -1, 'OM_AVG(acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, AVG(M1.acc_value) AS expr1 FROM incomeByLow_a_2012106 M1 WHERE M1.acc_id = 1 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (106, 2, 'expr2', 2, 'OM_AVG(acc0 * acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, AVG(M1.acc_value * A1.acc1) AS expr2 FROM incomeByLow_a_2012106 M1 INNER JOIN (SELECT run_id, dim0, dim1, dim2, dim3, dim4, sub_id, acc_value AS acc1 FROM incomeByLow_a_2012106 WHERE acc_id = 1) A1 ON (A1.run_id = M1.run_id AND A1.dim0 = M1.dim0 AND A1.dim1 = M1.dim1 AND A1.dim2 = M1.dim2 AND A1.dim3 = M1.dim3 AND A1.dim4 = M1.dim4 AND A1.sub_id = M1.sub_id) WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (106, 3, 'expr3', 2, 'OM_SUM(acc0 + acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, SUM(M1.acc_value + A1.acc1) AS expr3 FROM incomeByLow_a_2012106 M1 INNER JOIN (SELECT run_id, dim0, dim1, dim2, dim3, dim4, sub_id, acc_value AS acc1 FROM incomeByLow_a_2012106 WHERE acc_id = 1) A1 ON (A1.run_id = M1.run_id AND A1.dim0 = M1.dim0 AND A1.dim1 = M1.dim1 AND A1.dim2 = M1.dim2 AND A1.dim3 = M1.dim3 AND A1.dim4 = M1.dim4 AND A1.sub_id = M1.sub_id) WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (106, 0, 0, 'Average acc0', 'Average on acc0 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (106, 1, 0, 'Average acc1', 'Average on acc1 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (106, 2, 0, 'Average acc0 * acc1', 'Average on acc0 * acc1 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (106, 3, 0, 'Sum acc0 + acc1', 'Sum of acc0 + acc1 notes');

--
-- incomeByMiddle rank 5 [age, sex, salary, years, middle] = 144720 rows
--
INSERT INTO table_dic 
  (table_hid, table_name, table_digest, table_rank, is_sparse, db_expr_table, db_acc_table, db_acc_all_view, import_digest) 
VALUES 
  (107, 'incomeByMiddle', '_20128171604590107', 5, 0, 'incomeByMiddle_v_2012107', 'incomeByMiddle_a_2012107', 'incomeByMiddle_d_2012107', '_i20128171604590107');

INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos, is_hidden) VALUES (1, 6, 107, 0, 0, 0);

INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (107, 0, 'Income by Middle Period', 'Income by age, sex, salary, years, middle period notes', 'Income Measure', 'Income by Middle Period Measure notes');
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (107, 1, 'Revenu par période intermédiaire', 'Revenu par âge, sexe, salaire, années, période intermédiaire notes', 'Revenu par mesure', 'Notes sur la mesure du revenu par période intermédiaire');

-- total enum disabled for all dimensions
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (107, 0, 'dim0', 101, 0, 4);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (107, 1, 'dim1', 102, 0, 2);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (107, 2, 'dim2', 103, 0, 3);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (107, 3, 'dim3', 105, 0, 201);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (107, 4, 'dim4', 108, 0, 30);

INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (107, 0, 0, 'Age', 'Age Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (107, 0, 1, 'Âge', 'Âge Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (107, 1, 0, 'Sex', 'Sex Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (107, 1, 1, 'Sexe', NULL);
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (107, 2, 0, 'Salary', 'Salary Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (107, 2, 1, 'Salaire', 'Salaire Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (107, 3, 0, 'Years', 'Years of experience Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (107, 3, 1, 'Années', 'Des années d''expérience Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (107, 4, 0, 'Middle Period', 'Middle Period Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (107, 4, 1, 'Période Moyenne', 'Période Moyenne Dim notes');

INSERT INTO table_acc
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql)
VALUES
  (
  107, 0, 'acc0', 0, 'raw_value()',
  'SELECT run_id, sub_id, dim0, dim1, dim2, dim3, dim4, acc_value FROM incomeByMiddle_a_2012107 WHERE acc_id = 0'
  );

INSERT INTO table_acc 
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
VALUES 
  (
  107, 1, 'acc1', 0, 'acc1', 
  'SELECT run_id, sub_id, dim0, dim1, dim2, dim3, dim4, acc_value FROM incomeByMiddle_a_2012107 WHERE acc_id = 1'
  );

INSERT INTO table_acc 
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
VALUES 
  (
  107, 2, 'expr2', 1, 'acc0 - acc1', 
  '(A.acc_value - A1.acc_value)'
  );

INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (107, 0, 0, 'Income', 'Income notes');
INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (107, 1, 0, 'Income adjusted', 'Income adjusted notes');

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (107, 0, 'expr0', 2, 'OM_AVG(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, AVG(M1.acc_value) AS expr0 FROM incomeByMiddle_a_2012107 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (107, 1, 'expr1', -1, 'OM_AVG(acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, AVG(M1.acc_value) AS expr1 FROM incomeByMiddle_a_2012107 M1 WHERE M1.acc_id = 1 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (107, 2, 'expr2', 2, 'OM_AVG(acc0 * acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, AVG(M1.acc_value * A1.acc1) AS expr2 FROM incomeByMiddle_a_2012107 M1 INNER JOIN (SELECT run_id, dim0, dim1, dim2, dim3, dim4, sub_id, acc_value AS acc1 FROM incomeByMiddle_a_2012107 WHERE acc_id = 1) A1 ON (A1.run_id = M1.run_id AND A1.dim0 = M1.dim0 AND A1.dim1 = M1.dim1 AND A1.dim2 = M1.dim2 AND A1.dim3 = M1.dim3 AND A1.dim4 = M1.dim4 AND A1.sub_id = M1.sub_id) WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (107, 3, 'expr3', 2, 'OM_SUM(acc0 + acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, SUM(M1.acc_value + A1.acc1) AS expr3 FROM incomeByMiddle_a_2012107 M1 INNER JOIN (SELECT run_id, dim0, dim1, dim2, dim3, dim4, sub_id, acc_value AS acc1 FROM incomeByMiddle_a_2012107 WHERE acc_id = 1) A1 ON (A1.run_id = M1.run_id AND A1.dim0 = M1.dim0 AND A1.dim1 = M1.dim1 AND A1.dim2 = M1.dim2 AND A1.dim3 = M1.dim3 AND A1.dim4 = M1.dim4 AND A1.sub_id = M1.sub_id) WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (107, 0, 0, 'Average acc0', 'Average on acc0 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (107, 1, 0, 'Average acc1', 'Average on acc1 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (107, 2, 0, 'Average acc0 * acc1', 'Average on acc0 * acc1 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (107, 3, 0, 'Sum acc0 + acc1', 'Sum of acc0 + acc1 notes');

--
-- incomeByPeriod rank 5 [age, sex, salary, years, period] = 969624 rows
--
INSERT INTO table_dic 
  (table_hid, table_name, table_digest, table_rank, is_sparse, db_expr_table, db_acc_table, db_acc_all_view, import_digest) 
VALUES 
  (108, 'incomeByPeriod', '_20128171604590108', 5, 0, 'incomeByPeriod_v_2012108', 'incomeByPeriod_a_2012108', 'incomeByPeriod_d_2012108', '_i20128171604590108');

INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos, is_hidden) VALUES (1, 7, 108, 0, 0, 0);

INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (108, 0, 'Income by Period', 'Income by age, sex, salary, years, period notes', 'Income Measure', 'Income by Period Measure notes');
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (108, 1, 'Revenu par période', 'Revenu par âge, sexe, salaire, années, période notes', 'Revenu par mesure', 'Notes sur la mesure du revenu par période');

-- total enum disabled for all dimensions
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (108, 0, 'dim0', 101, 0, 4);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (108, 1, 'dim1', 102, 0, 2);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (108, 2, 'dim2', 103, 0, 3);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (108, 3, 'dim3', 105, 0, 201);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (108, 4, 'dim4', 106, 0, 201);

INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (108, 0, 0, 'Age', 'Age Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (108, 0, 1, 'Âge', 'Âge Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (108, 1, 0, 'Sex', 'Sex Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (108, 1, 1, 'Sexe', NULL);
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (108, 2, 0, 'Salary', 'Salary Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (108, 2, 1, 'Salaire', 'Salaire Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (108, 3, 0, 'Years', 'Years of experience Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (108, 3, 1, 'Années', 'Des années d''expérience Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (108, 4, 0, 'Period', 'Period Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (108, 4, 1, 'Période', 'Période Dim notes');

INSERT INTO table_acc
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql)
VALUES
  (
  108, 0, 'acc0', 0, 'raw_value()',
  'SELECT run_id, sub_id, dim0, dim1, dim2, dim3, dim4, acc_value FROM incomeByPeriod_a_2012108 WHERE acc_id = 0'
  );

INSERT INTO table_acc 
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
VALUES 
  (
  108, 1, 'acc1', 0, 'acc1', 
  'SELECT run_id, sub_id, dim0, dim1, dim2, dim3, dim4, acc_value FROM incomeByPeriod_a_2012108 WHERE acc_id = 1'
  );

INSERT INTO table_acc 
  (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
VALUES 
  (
  108, 2, 'expr2', 1, 'acc0 - acc1',
  '(A.acc_value - A1.acc_value)'
  );

INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (108, 0, 0, 'Income', 'Income notes');
INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (108, 1, 0, 'Income adjusted', 'Income adjusted notes');

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (108, 0, 'expr0', 2, 'OM_AVG(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, AVG(M1.acc_value) AS expr0 FROM incomeByPeriod_a_2012108 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (108, 1, 'expr1', -1, 'OM_AVG(acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, AVG(M1.acc_value) AS expr1 FROM incomeByPeriod_a_2012108 M1 WHERE M1.acc_id = 1 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (108, 2, 'expr2', 2, 'OM_AVG(acc0 * acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, AVG(M1.acc_value * A1.acc1) AS expr2 FROM incomeByPeriod_a_2012108 M1 INNER JOIN (SELECT run_id, dim0, dim1, dim2, dim3, dim4, sub_id, acc_value AS acc1 FROM incomeByPeriod_a_2012108 WHERE acc_id = 1) A1 ON (A1.run_id = M1.run_id AND A1.dim0 = M1.dim0 AND A1.dim1 = M1.dim1 AND A1.dim2 = M1.dim2 AND A1.dim3 = M1.dim3 AND A1.dim4 = M1.dim4 AND A1.sub_id = M1.sub_id) WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (108, 3, 'expr3', 2, 'OM_SUM(acc0 + acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4, SUM(M1.acc_value + A1.acc1) AS expr3 FROM incomeByPeriod_a_2012108 M1 INNER JOIN (SELECT run_id, dim0, dim1, dim2, dim3, dim4, sub_id, acc_value AS acc1 FROM incomeByPeriod_a_2012108 WHERE acc_id = 1) A1 ON (A1.run_id = M1.run_id AND A1.dim0 = M1.dim0 AND A1.dim1 = M1.dim1 AND A1.dim2 = M1.dim2 AND A1.dim3 = M1.dim3 AND A1.dim4 = M1.dim4 AND A1.sub_id = M1.sub_id) WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1, M1.dim2, M1.dim3, M1.dim4'
  );

INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (108, 0, 0, 'Average acc0', 'Average on acc0 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (108, 1, 0, 'Average acc1', 'Average on acc1 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (108, 2, 0, 'Average acc0 * acc1', 'Average on acc0 * acc1 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (108, 3, 0, 'Sum acc0 + acc1', 'Sum of acc0 + acc1 notes');


--
-- model groups of parameters
--
INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden) VALUES (1, 4, 1, 'LargeParameters',  1);

INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 4, 0, 'Large parameters', NULL);
INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 4, 1, 'Grands paramètres', NULL);

INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 1, 2,  4, NULL);


INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 3, 3, NULL, 9);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 3, 4, NULL, 10);

INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 4, 0, NULL, 7);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 4, 1, NULL, 8);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 4, 2, NULL, 9);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 4, 3, NULL, 10);

--
-- model groups of output tables
--
INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden) VALUES (1, 20, 0, 'LargeTables', 1);

INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 20, 0, 'Large output tables', 'Large output tables group notes');
INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (1, 20, 1, 'Grandes tables de sortie', 'Notes de groupe de grandes tables de sortie');

INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 10, 3, NULL, 4);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 20, 0, NULL, 5);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 20, 1, NULL, 6);
INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1, 20, 2, NULL, 7);


--
-- modelOne input parameters
--
-- enum ids for ageSex.dim0:     10,  20,  30, 40
-- enum ids for ageSex.dim1:     0,   1
-- enum ids for salaryAge.dim0:  100, 200, 300
-- enum ids for salaryAge.dim1:  10,  20,  30, 40
-- enum ids for isOldAge.dim0:   10,  20,  30, 40
-- enum ids for salaryFull.dim0: 100, 200, 300
-- enum ids for salaryFull.param_value: 22, 33
-- enum ids for baseSalary.param_value: 22, 33
--
-- enum ids for salaryByYears.dim0: 10,  20,  30, 40
-- enum ids for salaryByYears.dim1: 0,   1
-- enum ids for salaryByYears.dim2: 100, 200, 300
-- enum ids for salaryByYears.dim3: 0, 1, 2, 3, 4, ...., 200
--
-- enum ids for salaryByPeriod.dim0: 10,  20,  30, 40
-- enum ids for salaryByPeriod.dim1: 0,   1
-- enum ids for salaryByPeriod.dim2: 100, 200, 300
-- enum ids for salaryByPeriod.dim3: 0, 1, 2, 3, 4, ...., 200  = 201 item
-- enum ids for salaryByPeriod.dim4: 1, 3, 5, 7, 9, ...., 401  = 201 item
--
-- enum ids for salaryByLow.dim0: 10,  20,  30, 40
-- enum ids for salaryByLow.dim1: 0,   1
-- enum ids for salaryByLow.dim2: 100, 200, 300
-- enum ids for salaryByLow.dim3: 0, 1, 2, 3, 4, ...., 200
-- enum ids for salaryByLow.dim4: 1, 3, 5, 7, 9, ...., 19      = 10 items
--
-- enum ids for salaryByMiddle.dim0: 10,  20,  30, 40
-- enum ids for salaryByMiddle.dim1: 0,   1
-- enum ids for salaryByMiddle.dim2: 100, 200, 300
-- enum ids for salaryByMiddle.dim3: 0, 1, 2, 3, 4, ...., 200
-- enum ids for salaryByMiddle.dim4: 21, 23, 25, ...., 79      = 30 items
--

CREATE TABLE salaryByYears_p_2012818
(
  run_id      INT      NOT NULL,
  sub_id      SMALLINT NOT NULL,
  dim0        INT      NOT NULL,
  dim1        INT      NOT NULL,
  dim2        INT      NOT NULL,
  dim3        INT      NOT NULL,
  param_value FLOAT    NULL,     -- float parameter value can be null
  PRIMARY KEY (run_id, sub_id, dim0, dim1, dim2, dim3)
);

CREATE TABLE salaryByYears_w_2012818
(
  set_id      INT      NOT NULL,
  sub_id      SMALLINT NOT NULL,
  dim0        INT      NOT NULL,
  dim1        INT      NOT NULL,
  dim2        INT      NOT NULL,
  dim3        INT      NOT NULL,
  param_value FLOAT    NULL,     -- float parameter value can be null
  PRIMARY KEY (set_id, sub_id, dim0, dim1, dim2, dim3)
);

CREATE TABLE salaryByPeriod_p_2012819
(
  run_id      INT      NOT NULL,
  sub_id      SMALLINT NOT NULL,
  dim0        INT      NOT NULL,
  dim1        INT      NOT NULL,
  dim2        INT      NOT NULL,
  dim3        INT      NOT NULL,
  dim4        INT      NOT NULL,
  param_value FLOAT    NULL,     -- float parameter value can be null
  PRIMARY KEY (run_id, sub_id, dim0, dim1, dim2, dim3, dim4)
);

CREATE TABLE salaryByPeriod_w_2012819
(
  set_id      INT      NOT NULL,
  sub_id      SMALLINT NOT NULL,
  dim0        INT      NOT NULL,
  dim1        INT      NOT NULL,
  dim2        INT      NOT NULL,
  dim3        INT      NOT NULL,
  dim4        INT      NOT NULL,
  param_value FLOAT    NULL,     -- float parameter value can be null
  PRIMARY KEY (set_id, sub_id, dim0, dim1, dim2, dim3, dim4)
);

CREATE TABLE salaryByLow_p_201281_10
(
  run_id      INT      NOT NULL,
  sub_id      SMALLINT NOT NULL,
  dim0        INT      NOT NULL,
  dim1        INT      NOT NULL,
  dim2        INT      NOT NULL,
  dim3        INT      NOT NULL,
  dim4        INT      NOT NULL,
  param_value FLOAT    NULL,     -- float parameter value can be null
  PRIMARY KEY (run_id, sub_id, dim0, dim1, dim2, dim3, dim4)
);

CREATE TABLE salaryByLow_w_201281_10
(
  set_id      INT      NOT NULL,
  sub_id      SMALLINT NOT NULL,
  dim0        INT      NOT NULL,
  dim1        INT      NOT NULL,
  dim2        INT      NOT NULL,
  dim3        INT      NOT NULL,
  dim4        INT      NOT NULL,
  param_value FLOAT    NULL,     -- float parameter value can be null
  PRIMARY KEY (set_id, sub_id, dim0, dim1, dim2, dim3, dim4)
);

CREATE TABLE salaryByMiddle_p_201281_11
(
  run_id      INT      NOT NULL,
  sub_id      SMALLINT NOT NULL,
  dim0        INT      NOT NULL,
  dim1        INT      NOT NULL,
  dim2        INT      NOT NULL,
  dim3        INT      NOT NULL,
  dim4        INT      NOT NULL,
  param_value FLOAT    NULL,     -- float parameter value can be null
  PRIMARY KEY (run_id, sub_id, dim0, dim1, dim2, dim3, dim4)
);

CREATE TABLE salaryByMiddle_w_201281_11
(
  set_id      INT      NOT NULL,
  sub_id      SMALLINT NOT NULL,
  dim0        INT      NOT NULL,
  dim1        INT      NOT NULL,
  dim2        INT      NOT NULL,
  dim3        INT      NOT NULL,
  dim4        INT      NOT NULL,
  param_value FLOAT    NULL,     -- float parameter value can be null
  PRIMARY KEY (set_id, sub_id, dim0, dim1, dim2, dim3, dim4)
);

--
-- modelOne output tables
--

--
-- incomeByYear output table: [age, sex, salary, years] = 4824 rows
--
-- enum ids for incomeByYear.dim0: 10,  20,  30, 40
-- enum ids for incomeByYear.dim1: 0,   1
-- enum ids for incomeByYear.dim2: 100, 200, 300
-- enum ids for incomeByYear.dim3: 0, 1, 2, 3, 4, ...., 200  = 201 item
--
CREATE TABLE incomeByYear_a_2012105
(
  run_id    INT      NOT NULL,
  acc_id    SMALLINT NOT NULL,
  sub_id    SMALLINT NOT NULL,
  dim0      INT      NOT NULL,
  dim1      INT      NOT NULL,
  dim2      INT      NOT NULL,
  dim3      INT      NOT NULL,
  acc_value FLOAT    NULL,
  PRIMARY KEY (run_id, acc_id, sub_id, dim0, dim1, dim2, dim3)
);

CREATE TABLE incomeByYear_v_2012105
(
  run_id    INT      NOT NULL,
  expr_id   SMALLINT NOT NULL,
  dim0      INT      NOT NULL,
  dim1      INT      NOT NULL,
  dim2      INT      NOT NULL,
  dim3      INT      NOT NULL,
  expr_value FLOAT   NULL,
  PRIMARY KEY (run_id, expr_id, dim0, dim1, dim2, dim3)
);

--
-- incomeByLow output table: [age, sex, salary, years, low] = 48240 rows
--
-- enum ids for incomeByLow.dim0: 10,  20,  30, 40
-- enum ids for incomeByLow.dim1: 0,   1
-- enum ids for incomeByLow.dim2: 100, 200, 300
-- enum ids for incomeByLow.dim3: 0, 1, 2, 3, 4, ...., 200  = 201 item
-- enum ids for incomeByLow.dim4: 1, 3, 5, 7, 9, ...., 19   = 10 items
--
CREATE TABLE incomeByLow_a_2012106
(
  run_id    INT      NOT NULL,
  acc_id    SMALLINT NOT NULL,
  sub_id    SMALLINT NOT NULL,
  dim0      INT      NOT NULL,
  dim1      INT      NOT NULL,
  dim2      INT      NOT NULL,
  dim3      INT      NOT NULL,
  dim4      INT      NOT NULL,
  acc_value FLOAT    NULL,
  PRIMARY KEY (run_id, acc_id, sub_id, dim0, dim1, dim2, dim3, dim4)
);

CREATE TABLE incomeByLow_v_2012106
(
  run_id    INT      NOT NULL,
  expr_id   SMALLINT NOT NULL,
  dim0      INT      NOT NULL,
  dim1      INT      NOT NULL,
  dim2      INT      NOT NULL,
  dim3      INT      NOT NULL,
  dim4      INT      NOT NULL,
  expr_value FLOAT   NULL,
  PRIMARY KEY (run_id, expr_id, dim0, dim1, dim2, dim3, dim4)
);

--
-- incomeByMiddle output table: [age, sex, salary, years, middle] = 144720 rows
--
-- enum ids for incomeByMiddle.dim0: 10,  20,  30, 40
-- enum ids for incomeByMiddle.dim1: 0,   1
-- enum ids for incomeByMiddle.dim2: 100, 200, 300
-- enum ids for incomeByMiddle.dim3: 0, 1, 2, 3, 4, ...., 200  = 201 item
-- enum ids for incomeByMiddle.dim4: 21, 23, 25, ...., 79      = 30 items
--
CREATE TABLE incomeByMiddle_a_2012107
(
  run_id    INT      NOT NULL,
  acc_id    SMALLINT NOT NULL,
  sub_id    SMALLINT NOT NULL,
  dim0      INT      NOT NULL,
  dim1      INT      NOT NULL,
  dim2      INT      NOT NULL,
  dim3      INT      NOT NULL,
  dim4      INT      NOT NULL,
  acc_value FLOAT    NULL,
  PRIMARY KEY (run_id, acc_id, sub_id, dim0, dim1, dim2, dim3, dim4)
);

CREATE TABLE incomeByMiddle_v_2012107
(
  run_id     INT      NOT NULL,
  expr_id    SMALLINT NOT NULL,
  dim0       INT      NOT NULL,
  dim1       INT      NOT NULL,
  dim2       INT      NOT NULL,
  dim3       INT      NOT NULL,
  dim4       INT      NOT NULL,
  expr_value FLOAT    NULL,
  PRIMARY KEY (run_id, expr_id, dim0, dim1, dim2, dim3, dim4)
);

--
-- incomeByPeriod output table: [age, sex, salary, years, period] = 969624 rows
--
-- enum ids for incomeByPeriod.dim0: 10,  20,  30, 40
-- enum ids for incomeByPeriod.dim1: 0,   1
-- enum ids for incomeByPeriod.dim2: 100, 200, 300
-- enum ids for incomeByPeriod.dim3: 0, 1, 2, 3, 4, ...., 200  = 201 item
-- enum ids for incomeByPeriod.dim4: 1, 3, 5, 7, 9, ...., 401  = 201 item
--
CREATE TABLE incomeByPeriod_a_2012108
(
  run_id    INT      NOT NULL,
  acc_id    SMALLINT NOT NULL,
  sub_id    SMALLINT NOT NULL,
  dim0      INT      NOT NULL,
  dim1      INT      NOT NULL,
  dim2      INT      NOT NULL,
  dim3      INT      NOT NULL,
  dim4      INT      NOT NULL,
  acc_value FLOAT    NULL,
  PRIMARY KEY (run_id, acc_id, sub_id, dim0, dim1, dim2, dim3, dim4)
);

CREATE TABLE incomeByPeriod_v_2012108
(
  run_id     INT      NOT NULL,
  expr_id    SMALLINT NOT NULL,
  dim0       INT      NOT NULL,
  dim1       INT      NOT NULL,
  dim2       INT      NOT NULL,
  dim3       INT      NOT NULL,
  dim4       INT      NOT NULL,
  expr_value FLOAT    NULL,
  PRIMARY KEY (run_id, expr_id, dim0, dim1, dim2, dim3, dim4)
);


--
-- modelOne all accumulators view
--

-- if MSSQL return an error on CREATE VIEW then uncomment next line GO
--
-- GO

--
-- incomeByYear all accumulators view
--
-- it does include all "native" accumulators: acc0, acc1
-- and "derived" accumulator: acc2 = acc0 - acc1
--
CREATE VIEW incomeByYear_d_2012105
AS
SELECT
  A.run_id,
  A.sub_id,
  A.dim0,
  A.dim1,
  A.dim2,
  A.dim3,
  A.acc_value AS acc0,
  (
    SELECT A1.acc_value FROM incomeByYear_a_2012105 A1
    WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1 AND A1.dim2 = A.dim2 AND A1.dim3 = A.dim3
    AND A1.acc_id = 1
  ) AS acc1,
  (
    (
      A.acc_value
    )
    - 
    (
      SELECT A1.acc_value FROM incomeByYear_a_2012105 A1
      WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1 AND A1.dim2 = A.dim2 AND A1.dim3 = A.dim3
      AND A1.acc_id = 1
    )
  ) AS acc2
FROM incomeByYear_a_2012105 A
WHERE A.acc_id = 0;

--
-- incomeByLow all accumulators view
--
-- it does include all "native" accumulators: acc0, acc1
-- and "derived" accumulator: acc2 = acc0 - acc1
--
CREATE VIEW incomeByLow_d_2012106
AS
SELECT
  A.run_id,
  A.sub_id,
  A.dim0,
  A.dim1,
  A.dim2,
  A.dim3,
  A.dim4,
  A.acc_value AS acc0,
  (
    SELECT A1.acc_value FROM incomeByLow_a_2012106 A1
    WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1 AND A1.dim2 = A.dim2 AND A1.dim3 = A.dim3 AND A1.dim4 = A.dim4
    AND A1.acc_id = 1
  ) AS acc1,
  (
    (
      A.acc_value
    )
    - 
    (
      SELECT A1.acc_value FROM incomeByLow_a_2012106 A1
      WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1 AND A1.dim2 = A.dim2 AND A1.dim3 = A.dim3 AND A1.dim4 = A.dim4
      AND A1.acc_id = 1
    )
  ) AS acc2
FROM incomeByLow_a_2012106 A
WHERE A.acc_id = 0;

--
-- incomeByMiddle all accumulators view
--
-- it does include all "native" accumulators: acc0, acc1
-- and "derived" accumulator: acc2 = acc0 - acc1
--
CREATE VIEW incomeByMiddle_d_2012107
AS
SELECT
  A.run_id,
  A.sub_id,
  A.dim0,
  A.dim1,
  A.dim2,
  A.dim3,
  A.dim4,
  A.acc_value AS acc0,
  (
    SELECT A1.acc_value FROM incomeByMiddle_a_2012107 A1
    WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1 AND A1.dim2 = A.dim2 AND A1.dim3 = A.dim3 AND A1.dim4 = A.dim4
    AND A1.acc_id = 1
  ) AS acc1,
  (
    (
      A.acc_value
    )
    - 
    (
      SELECT A1.acc_value FROM incomeByMiddle_a_2012107 A1
      WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1 AND A1.dim2 = A.dim2 AND A1.dim3 = A.dim3 AND A1.dim4 = A.dim4
      AND A1.acc_id = 1
    )
  ) AS acc2
FROM incomeByMiddle_a_2012107 A
WHERE A.acc_id = 0;

--
-- incomeByPeriod all accumulators view
--
-- it does include all "native" accumulators: acc0, acc1
-- and "derived" accumulator: acc2 = acc0 - acc1
--
CREATE VIEW incomeByPeriod_d_2012108
AS
SELECT
  A.run_id,
  A.sub_id,
  A.dim0,
  A.dim1,
  A.dim2,
  A.dim3,
  A.dim4,
  A.acc_value AS acc0,
  (
    SELECT A1.acc_value FROM incomeByPeriod_a_2012108 A1
    WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1 AND A1.dim2 = A.dim2 AND A1.dim3 = A.dim3 AND A1.dim4 = A.dim4
    AND A1.acc_id = 1
  ) AS acc1,
  (
    (
      A.acc_value
    )
    - 
    (
      SELECT A1.acc_value FROM incomeByPeriod_a_2012108 A1
      WHERE A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1 AND A1.dim2 = A.dim2 AND A1.dim3 = A.dim3 AND A1.dim4 = A.dim4
      AND A1.acc_id = 1
    )
  ) AS acc2
FROM incomeByPeriod_a_2012108 A
WHERE A.acc_id = 0;
