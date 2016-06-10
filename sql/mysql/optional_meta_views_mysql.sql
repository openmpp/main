--
-- Copyright (c) 2013 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--

--
-- Compatibility views
--
-- Dear user:
-- this part of database is optional and NOT used by openM++
-- if you want to have it for any reason please enjoy
-- if you don't need it then ignore everything below
--

CREATE VIEW LanguageDic AS
SELECT 
  L.lang_id   AS "LanguageID", 
  L.lang_code AS "LanguageCode", 
  L.lang_name AS "LanguageName", 
  (SELECT LWA.word_value FROM lang_word LWA WHERE LWA.lang_id = L.lang_id AND LWA.word_code = 'all') AS "All", 
  (SELECT LWN.word_value FROM lang_word LWN WHERE LWN.lang_id = L.lang_id AND LWN.word_code = 'min') AS "Min", 
  (SELECT LWX.word_value FROM lang_word LWX WHERE LWX.lang_id = L.lang_id AND LWX.word_code = 'max') AS "Max"
FROM lang_lst L;

CREATE VIEW ModelDic AS
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

CREATE VIEW ModelInfoDic AS
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
    SELECT RO1.option_value FROM run_option RO1 WHERE RO1.run_id = R.run_id AND RO1.option_key = 'Parameter.Cases'
  ) AS "Cases",
  (
    SELECT RO2.option_value FROM run_option RO2 WHERE RO2.run_id = R.run_id AND RO2.option_key = 'Parameter.Cases'
  ) AS "CasesRequested",
  RT.lang_id   AS "LanguageID" 
FROM model_dic M
INNER JOIN run_lst R ON (R.model_id = M.model_id)
INNER JOIN run_txt RT ON (RT.run_id = R.run_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND R.run_id = (SELECT MIN(FR.run_id) FROM run_lst FR WHERE FR.model_id = M.model_id);

CREATE VIEW ScenarioDic AS
SELECT
  R.create_dt  AS "Name", 
  RT.descr     AS "Description", 
  RT.note      AS "Note", 
  R.sub_count  AS "Subsamples",
  (
    SELECT RO1.option_value FROM run_option RO1 WHERE RO1.run_id = R.run_id AND RO1.option_key = 'Parameter.Cases'
  ) AS "Cases",
  (
    SELECT RO2.option_value FROM run_option RO2 WHERE RO2.run_id = R.run_id AND RO2.option_key = 'Parameter.Seed'
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

CREATE VIEW SimulationInfoDic AS SELECT * FROM ModelInfoDic;

CREATE VIEW TypeDic AS
SELECT
  M.model_type_id AS "TypeID", 
  L.dic_id        AS "DicID"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE VIEW SimpleTypeDic AS
SELECT
  M.model_type_id AS "TypeID", 
  L.type_name     AS "Name"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND L.dic_id = 0;

CREATE VIEW LogicalDic AS
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

CREATE VIEW ClassificationDic AS
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

CREATE VIEW ClassificationValueDic AS
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

CREATE VIEW RangeDic AS
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

CREATE VIEW RangeValueDic AS
SELECT
  M.model_type_id AS "TypeID", 
  E.enum_name     AS "Value"
FROM type_dic L
INNER JOIN model_type_dic M ON (M.type_hid = L.type_hid)
INNER JOIN type_enum_lst E ON (E.type_hid = M.type_hid)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND L.dic_id = 3;

CREATE VIEW PartitionDic AS
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

CREATE VIEW PartitionValueDic AS
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

CREATE VIEW PartitionIntervalDic AS
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

-- CREATE VIEW LinkTypeDic AS
-- SELECT
--   M.model_type_id AS "TypeID", 
--   M.type_name     AS "Name",
--   NULL            AS "LinkedActorID"
-- FROM type_dic M
-- WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
-- AND M.dic_id = 5;

CREATE VIEW ParameterDic AS
SELECT
  M.model_parameter_id AS "ParameterID", 
  L.parameter_name     AS "Name", 
  DT.descr             AS "Description",
  DT.note              AS "Note",
  RT.note              AS "ValueNote",
  MTD.model_type_id    AS "TypeID", 
  L.parameter_rank     AS "Rank",
  L.num_cumulated      AS "NumberOfCumulatedDimensions",
  M.is_generated       AS "ModelGenerated",
  M.is_hidden          AS "Hidden",
  DT.lang_id           AS "LanguageID" 
FROM parameter_dic L
INNER JOIN model_parameter_dic M ON (M.parameter_hid = L.parameter_hid)
INNER JOIN parameter_dic_txt DT ON (DT.parameter_hid = M.parameter_hid)
INNER JOIN model_type_dic MTD ON (MTD.type_hid = L.type_hid)
LEFT OUTER JOIN run_parameter_txt RT ON (RT.parameter_hid = DT.parameter_hid AND RT.lang_id = DT.lang_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND RT.run_id = (SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = M.model_id);

CREATE VIEW ParameterDimensionDic AS
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

CREATE VIEW ParameterGroupDic AS
SELECT
  M.group_id     AS "ParameterGroupID", 
  M.group_name   AS "Name", 
  GT.descr       AS "Description",
  GT.note        AS "Note",
  M.is_generated AS "ModelGenerated", 
  M.is_hidden    AS "Hidden", 
  GT.lang_id     AS "LanguageID" 
FROM group_lst M
INNER JOIN group_txt GT ON (GT.model_id = M.model_id AND GT.group_id = M.group_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_parameter <> 0;

CREATE VIEW ParameterGroupMemberDic AS
SELECT
  M.group_id        AS "ParameterGroupID", 
  PC.child_pos      AS "Position", 
  PC.leaf_id        AS "ParameterID",
  PC.child_group_id AS "MemberGroupID" 
FROM group_lst M
INNER JOIN group_pc PC ON (PC.model_id = M.model_id AND PC.group_id = M.group_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_parameter <> 0;

CREATE VIEW TableDic AS
SELECT
  M.model_table_id AS "TableID", 
  L.table_name     AS "Name", 
  T.descr          AS "Description",
  T.note           AS "Note",
  L.table_rank + 1 AS "Rank",
  M.expr_dim_pos   AS "AnalysisDimensionPosition", 
  CONCAT(L.table_name, '.DimA') AS "AnalysisDimensionName", 
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

CREATE VIEW UserTableDic AS
SELECT
  M.model_table_id AS "TableID", 
  L.table_name     AS "Name", 
  T.descr          AS "Description",
  T.note           AS "Note",
  L.table_rank + 1 AS "Rank",
  M.expr_dim_pos   AS "AnalysisDimensionPosition", 
  CONCAT(L.table_name, '.DimA') AS "AnalysisDimensionName", 
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

CREATE VIEW TableClassDic AS
SELECT
  M.model_table_id  AS "TableID", 
  D.dim_id          AS "Position",
  CONCAT(L.table_name, '.', D.dim_name) AS "Name", 
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

CREATE VIEW TableExpressionDic AS
SELECT
  M.model_table_id AS "TableID", 
  E.expr_id        AS "ExpressionID", 
  CONCAT(L.table_name, '.', E.expr_name) AS "Name", 
  T.descr          AS "Description",
  T.note           AS "Note",
  E.expr_decimals  AS "Decimals",
  T.lang_id        AS "LanguageID" 
FROM table_dic L
INNER JOIN model_table_dic M ON (M.table_hid = L.table_hid)
INNER JOIN table_expr E ON (E.table_hid = L.table_hid)
INNER JOIN table_expr_txt T ON (T.table_hid = M.table_hid AND T.expr_id = E.expr_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE VIEW TableGroupDic AS
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

CREATE VIEW TableGroupMemberDic AS
SELECT
  M.group_id        AS "TableGroupID", 
  PC.child_pos      AS "Position", 
  PC.leaf_id        AS "TableID",
  PC.child_group_id AS "MemberGroupID" 
FROM group_lst M
INNER JOIN group_pc PC ON (PC.model_id = M.model_id AND PC.group_id = M.group_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_parameter = 0;
