--
-- Copyright (c) 2013 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--
-- keep dummy sql below to prevent sqlite3 failure due to UTF-8 BOM
-- it is typical problem if .sql saved by Windows text editors
--
SELECT * FROM sqlite_master WHERE 0 = 1;

--
-- Compatibility views
--
-- Dear user:
-- this part of database is optional and NOT used by openM++
-- if you want to have it for any reason please enjoy
-- if you don't need it then ignore everything below
--

CREATE VIEW IF NOT EXISTS LanguageDic AS
SELECT 
  L.lang_id   AS "LanguageID", 
  L.lang_code AS "LanguageCode", 
  L.lang_name AS "LanguageName", 
  (SELECT LWA.word_value FROM lang_word LWA WHERE LWA.lang_id = L.lang_id AND LWA.word_code = 'all') AS "All", 
  (SELECT LWN.word_value FROM lang_word LWN WHERE LWN.lang_id = L.lang_id AND LWN.word_code = 'min') AS "Min", 
  (SELECT LWX.word_value FROM lang_word LWX WHERE LWX.lang_id = L.lang_id AND LWX.word_code = 'max') AS "Max"
FROM lang_lst L;

CREATE VIEW IF NOT EXISTS ModelDic AS
SELECT 
  M.model_name AS "Name", 
  T.descr      AS "Description", 
  T.note       AS "Note", 
  M.model_type AS "ModelType", 
  M.model_ver  AS "Version", 
  T.lang_id    AS "LanguageID" 
FROM model_dic M
INNER JOIN model_dic_txt T ON (T.model_id = M.model_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE VIEW IF NOT EXISTS ModelInfoDic AS
SELECT
  R.create_dt  AS "Time", 
  ''           AS "Directory", 
  ''           AS "CommandLine",
  ''           AS "CompletionStatus",
  R.sub_count  AS "Subsamples",
  0            AS "CV", 
  0            AS "SE",
  M.model_type AS "ModelType",
  -1           AS "FullReport",
  (
    SELECT RO1.option_value FROM run_option RO1 WHERE RO1.run_id = R.run_id AND RO1.option_key = 'Parameter.SimulationCases'
  ) AS "Cases",
  (
    SELECT RO2.option_value FROM run_option RO2 WHERE RO2.run_id = R.run_id AND RO2.option_key = 'Parameter.SimulationCases'
  ) AS "CasesRequested",
  RT.lang_id   AS "LanguageID" 
FROM model_dic M
INNER JOIN run_lst R ON (R.model_id = M.model_id)
INNER JOIN run_txt RT ON (RT.run_id = R.run_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND R.run_id = (SELECT MIN(FR.run_id) FROM run_lst FR WHERE FR.model_id = M.model_id);

CREATE VIEW IF NOT EXISTS ScenarioDic AS
SELECT
  R.run_name  AS "Name",
  RT.descr    AS "Description",
  RT.note     AS "Note",
  R.sub_count AS "Subsamples",
  (
    SELECT RO1.option_value FROM run_option RO1 WHERE RO1.run_id = R.run_id AND RO1.option_key = 'Parameter.SimulationCases'
  ) AS "Cases",
  (
    SELECT RO2.option_value FROM run_option RO2 WHERE RO2.run_id = R.run_id AND RO2.option_key = 'Parameter.SimulationSeed'
  ) AS "Seed",
  (
    SELECT RO3.option_value FROM run_option RO3 WHERE RO3.run_id = R.run_id AND RO3.option_key = 'Parameter.PopulationScaling'
  ) AS "PopulationScaling",
  (
    SELECT RO4.option_value FROM run_option RO4 WHERE RO4.run_id = R.run_id AND RO4.option_key = 'Parameter.Population'
  ) AS "PopulationSize",
  1            AS "CopyParameters", 
  RT.lang_id   AS "LanguageID" 
FROM model_dic M
INNER JOIN run_lst R ON (R.model_id = M.model_id)
INNER JOIN run_txt RT ON (RT.run_id = R.run_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND R.run_id = (SELECT MIN(FR.run_id) FROM run_lst FR WHERE FR.model_id = M.model_id);

CREATE VIEW IF NOT EXISTS SimulationInfoDic AS SELECT * FROM ModelInfoDic;

CREATE VIEW IF NOT EXISTS TypeDic AS
SELECT
  M.model_type_id AS "TypeID", 
  L.dic_id        AS "DicID"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE VIEW IF NOT EXISTS SimpleTypeDic AS
SELECT
  M.model_type_id AS "TypeID", 
  L.type_name     AS "Name"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND L.dic_id = 0;

CREATE VIEW IF NOT EXISTS LogicalDic AS
SELECT
  M.model_type_id AS "TypeID", 
  L.type_name     AS "Name",
  E.enum_id       AS "Value",
  E.enum_name     AS "ValueName",
  ET.descr        AS "ValueDescription",
  ET.lang_id      AS "LanguageID"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
INNER JOIN type_enum_lst E ON (E.type_hid = L.type_hid)
INNER JOIN type_enum_txt ET ON (ET.type_hid = E.type_hid AND ET.enum_id = E.enum_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND L.dic_id = 1;

CREATE VIEW IF NOT EXISTS ClassificationDic AS
SELECT
  M.model_type_id AS "TypeID", 
  L.type_name     AS "Name",
  MT.descr        AS "Description",
  MT.note         AS "Note",
  L.total_enum_id AS "NumberOfValues",
  MT.lang_id      AS "LanguageID"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
INNER JOIN type_dic_txt MT ON (MT.type_hid = L.type_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND L.dic_id = 2;

CREATE VIEW IF NOT EXISTS ClassificationValueDic AS
SELECT
  M.model_type_id AS "TypeID", 
  E.enum_id       AS "EnumValue",
  E.enum_name     AS "Name",
  ET.descr        AS "Description",
  ET.note         AS "Note",
  ET.lang_id      AS "LanguageID"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
INNER JOIN type_enum_lst E ON (E.type_hid = M.type_hid)
INNER JOIN type_enum_txt ET ON (ET.type_hid = E.type_hid AND ET.enum_id = E.enum_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND L.dic_id = 2;

CREATE VIEW IF NOT EXISTS RangeDic AS
SELECT
  M.model_type_id AS "TypeID", 
  L.type_name     AS "Name",
  MT.descr        AS "Description",
  MT.note         AS "Note",
  (
    SELECT MIN(EN.enum_name) FROM type_enum_lst EN WHERE EN.type_hid = M.type_hid
  )
  AS "Min",
  (
    SELECT MAX(EX.enum_name) FROM type_enum_lst EX WHERE EX.type_hid = M.type_hid
  )
  AS "Max",
  MT.lang_id      AS "LanguageID"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
INNER JOIN type_dic_txt MT ON (MT.type_hid = M.type_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND L.dic_id = 3;

CREATE VIEW IF NOT EXISTS RangeValueDic AS
SELECT
  M.model_type_id AS "TypeID", 
  E.enum_name     AS "Value"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
INNER JOIN type_enum_lst E ON (E.type_hid = M.type_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND L.dic_id = 3;

CREATE VIEW IF NOT EXISTS PartitionDic AS
SELECT
  M.model_type_id AS "TypeID", 
  L.type_name     AS "Name",
  MT.descr        AS "Description",
  MT.note         AS "Note",
  L.total_enum_id AS "NumberOfValues",
  MT.lang_id      AS "LanguageID"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
INNER JOIN type_dic_txt MT ON (MT.type_hid = M.type_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND L.dic_id = 4;

CREATE VIEW IF NOT EXISTS PartitionValueDic AS
SELECT
  M.model_type_id AS "TypeID", 
  E.enum_id       AS "Position",
  E.enum_name     AS "Value",
  E.enum_name     AS "StringValue"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
INNER JOIN type_enum_lst E ON (E.type_hid = M.type_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND L.dic_id = 4;

CREATE VIEW IF NOT EXISTS PartitionIntervalDic AS
SELECT
  M.model_type_id AS "TypeID", 
  E.enum_id       AS "Position",
  ET.descr        AS "Description",
  ET.lang_id      AS "LanguageID"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
INNER JOIN type_enum_lst E ON (E.type_hid = M.type_hid)
INNER JOIN type_enum_txt ET ON (ET.type_hid = E.type_hid AND ET.enum_id = E.enum_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND L.dic_id = 4;

-- CREATE VIEW IF NOT EXISTS LinkTypeDic AS
-- SELECT
--   M.model_type_id AS "TypeID", 
--   M.type_name     AS "Name",
--   NULL            AS "LinkedActorID"
-- FROM type_dic M
-- WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
-- AND M.dic_id = 5;

CREATE VIEW IF NOT EXISTS ParameterDic AS
SELECT
  M.model_parameter_id AS "ParameterID", 
  L.parameter_name     AS "Name", 
  DT.descr             AS "Description",
  DT.note              AS "Note",
  RT.note              AS "ValueNote",
  MTD.model_type_id    AS "TypeID", 
  L.parameter_rank     AS "Rank",
  L.num_cumulated      AS "NumberOfCumulatedDimensions",
  0                    AS "ModelGenerated",
  M.is_hidden          AS "Hidden",
  DT.lang_id           AS "LanguageID" 
FROM parameter_dic L
INNER JOIN model_parameter_dic M ON (M.parameter_hid = L.parameter_hid)
INNER JOIN parameter_dic_txt DT ON (DT.parameter_hid = M.parameter_hid)
INNER JOIN model_type_dic MTD ON (MTD.type_hid = L.type_hid)
LEFT OUTER JOIN run_parameter_txt RT ON
(
  RT.parameter_hid = DT.parameter_hid AND
  RT.lang_id = DT.lang_id AND
  RT.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = M.model_id)
)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE VIEW IF NOT EXISTS ParameterDimensionDic AS
SELECT
  M.model_parameter_id AS "ParameterID", 
  D.dim_id             AS "DisplayPosition", 
  MTD.model_type_id    AS "TypeID", 
  D.dim_id             AS "Position"
FROM parameter_dic L
INNER JOIN model_parameter_dic M ON (M.parameter_hid = L.parameter_hid)
INNER JOIN parameter_dims D ON (D.parameter_hid = M.parameter_hid)
INNER JOIN model_type_dic MTD ON (MTD.type_hid = L.type_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE VIEW IF NOT EXISTS ParameterGroupDic AS
SELECT
  M.group_id     AS "ParameterGroupID", 
  M.group_name   AS "Name", 
  GT.descr       AS "Description",
  GT.note        AS "Note",
  0              AS "ModelGenerated", 
  M.is_hidden    AS "Hidden", 
  GT.lang_id     AS "LanguageID" 
FROM group_lst M
INNER JOIN group_txt GT ON (GT.model_id = M.model_id AND GT.group_id = M.group_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_parameter <> 0;

CREATE VIEW IF NOT EXISTS ParameterGroupMemberDic AS
SELECT
  M.group_id        AS "ParameterGroupID", 
  PC.child_pos      AS "Position", 
  PC.leaf_id        AS "ParameterID",
  PC.child_group_id AS "MemberGroupID" 
FROM group_lst M
INNER JOIN group_pc PC ON (PC.model_id = M.model_id AND PC.group_id = M.group_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_parameter <> 0;

CREATE VIEW IF NOT EXISTS TableDic AS
SELECT
  M.model_table_id AS "TableID", 
  L.table_name     AS "Name", 
  T.descr          AS "Description",
  T.note           AS "Note",
  L.table_rank + 1 AS "Rank",
  M.expr_dim_pos   AS "AnalysisDimensionPosition", 
  L.table_name || '.DimA' AS "AnalysisDimensionName", 
  T.expr_descr     AS "AnalysisDimensionDescription",
  T.expr_note      AS "AnalysisDimensionNote",
  L.is_sparse      AS "Sparse",
  0                AS "Hidden",
  T.lang_id        AS "LanguageID" 
FROM table_dic L
INNER JOIN model_table_dic M ON (M.table_hid = L.table_hid)
INNER JOIN table_dic_txt T ON (T.table_hid = M.table_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_user = 0;

CREATE VIEW IF NOT EXISTS UserTableDic AS
SELECT
  M.model_table_id AS "TableID", 
  L.table_name     AS "Name", 
  T.descr          AS "Description",
  T.note           AS "Note",
  L.table_rank + 1 AS "Rank",
  M.expr_dim_pos   AS "AnalysisDimensionPosition", 
  L.table_name || '.DimA' AS "AnalysisDimensionName", 
  T.expr_descr     AS "AnalysisDimensionDescription",
  T.expr_note      AS "AnalysisDimensionNote",
  L.is_sparse      AS "Sparse",
  0                AS "Hidden",
  T.lang_id        AS "LanguageID" 
FROM table_dic L
INNER JOIN model_table_dic M ON (M.table_hid = L.table_hid)
INNER JOIN table_dic_txt T ON (T.table_hid = M.table_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_user <> 0;

CREATE VIEW IF NOT EXISTS TableClassDic AS
SELECT
  M.model_table_id  AS "TableID", 
  D.dim_id          AS "Position",
  L.table_name || '.' || D.dim_name AS "Name", 
  T.descr           AS "Description",
  T.note            AS "Note",
  MTD.model_type_id AS "TypeID", 
  D.is_total        AS "Totals",
  T.lang_id         AS "LanguageID" 
FROM table_dic L
INNER JOIN model_table_dic M ON (M.table_hid = L.table_hid)
INNER JOIN table_dims D ON (D.table_hid = L.table_hid)
INNER JOIN model_type_dic MTD ON (MTD.type_hid = D.type_hid)
INNER JOIN table_dims_txt T ON (T.table_hid = D.table_hid AND T.dim_id = D.dim_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE VIEW IF NOT EXISTS TableExpressionDic AS
SELECT
  M.model_table_id AS "TableID", 
  E.expr_id        AS "ExpressionID", 
  L.table_name || '.' || E.expr_name AS "Name", 
  T.descr          AS "Description",
  T.note           AS "Note",
  E.expr_decimals  AS "Decimals",
  T.lang_id        AS "LanguageID" 
FROM table_dic L
INNER JOIN model_table_dic M ON (M.table_hid = L.table_hid)
INNER JOIN table_expr E ON (E.table_hid = L.table_hid)
INNER JOIN table_expr_txt T ON (T.table_hid = M.table_hid AND T.expr_id = E.expr_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE VIEW IF NOT EXISTS TableGroupDic AS
SELECT
  M.group_id     AS "TableGroupID", 
  M.group_name   AS "Name", 
  GT.descr       AS "Description",
  GT.note        AS "Note",
  M.is_hidden    AS "Hidden", 
  GT.lang_id     AS "LanguageID" 
FROM group_lst M
INNER JOIN group_txt GT ON (GT.model_id = M.model_id AND GT.group_id = M.group_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_parameter = 0;

CREATE VIEW IF NOT EXISTS TableGroupMemberDic AS
SELECT
  M.group_id        AS "TableGroupID", 
  PC.child_pos      AS "Position", 
  PC.leaf_id        AS "TableID",
  PC.child_group_id AS "MemberGroupID" 
FROM group_lst M
INNER JOIN group_pc PC ON (PC.model_id = M.model_id AND PC.group_id = M.group_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_parameter = 0;
