--
-- Copyright (c) 2014 OpenM++
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

CREATE OR REPLACE VIEW LanguageDic AS
SELECT 
  L.lang_id   AS "LanguageID", 
  L.lang_code AS "LanguageCode", 
  L.lang_name AS "LanguageName", 
  (SELECT LWA.word_value FROM lang_word LWA WHERE LWA.lang_id = L.lang_id AND LWA.word_code = 'all') AS "All", 
  (SELECT LWN.word_value FROM lang_word LWN WHERE LWN.lang_id = L.lang_id AND LWN.word_code = 'min') AS "Min", 
  (SELECT LWX.word_value FROM lang_word LWX WHERE LWX.lang_id = L.lang_id AND LWX.word_code = 'max') AS "Max"
FROM lang_lst L;

CREATE OR REPLACE VIEW ModelDic AS
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

CREATE OR REPLACE VIEW ModelInfoDic AS
SELECT
  R.create_dt  AS "Time", 
  CAST('' AS VARCHAR(255)) AS "Directory", 
  CAST('' AS VARCHAR(255)) AS "CommandLine",
  CAST('' AS VARCHAR(16))  AS "CompletionStatus",
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

CREATE OR REPLACE VIEW ScenarioDic AS
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

CREATE OR REPLACE VIEW SimulationInfoDic AS SELECT * FROM ModelInfoDic;

CREATE OR REPLACE VIEW TypeDic AS
SELECT
  M.mod_type_id AS "TypeID", 
  M.dic_id      AS "DicID"
FROM type_dic M
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE OR REPLACE VIEW SimpleTypeDic AS
SELECT
  M.mod_type_id    AS "TypeID", 
  M.mod_type_name  AS "Name"
FROM type_dic M
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.dic_id = 0;

CREATE OR REPLACE VIEW LogicalDic AS
SELECT
  M.mod_type_id   AS "TypeID", 
  M.mod_type_name AS "Name",
  E.enum_id       AS "Value",
  E.enum_name     AS "ValueName",
  ET.descr        AS "ValueDescription",
  ET.lang_id      AS "LanguageID"
FROM type_dic M
INNER JOIN type_enum_lst E ON (E.model_id = M.model_id AND E.mod_type_id = M.mod_type_id)
INNER JOIN type_enum_txt ET ON (ET.model_id = E.model_id AND ET.mod_type_id = E.mod_type_id AND ET.enum_id = E.enum_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.dic_id = 1;

CREATE OR REPLACE VIEW ClassificationDic AS
SELECT
  M.mod_type_id   AS "TypeID", 
  M.mod_type_name AS "Name",
  MT.descr        AS "Description",
  MT.note         AS "Note",
  M.total_enum_id AS "NumberOfValues",
  MT.lang_id      AS "LanguageID"
FROM type_dic M
INNER JOIN type_dic_txt MT ON (MT.model_id = M.model_id AND MT.mod_type_id = M.mod_type_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.dic_id = 2;

CREATE OR REPLACE VIEW ClassificationValueDic AS
SELECT
  M.mod_type_id   AS "TypeID", 
  E.enum_id       AS "EnumValue",
  E.enum_name     AS "Name",
  ET.descr        AS "Description",
  ET.note         AS "Note",
  ET.lang_id      AS "LanguageID"
FROM type_dic M
INNER JOIN type_enum_lst E ON (E.model_id = M.model_id AND E.mod_type_id = M.mod_type_id)
INNER JOIN type_enum_txt ET ON (ET.model_id = E.model_id AND ET.mod_type_id = E.mod_type_id AND ET.enum_id = E.enum_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.dic_id = 2;

CREATE OR REPLACE VIEW RangeDic AS
SELECT
  M.mod_type_id   AS "TypeID", 
  M.mod_type_name AS "Name",
  MT.descr        AS "Description",
  MT.note         AS "Note",
  (
    SELECT MIN(EN.enum_name) FROM type_enum_lst EN WHERE EN.model_id = M.model_id AND EN.mod_type_id = M.mod_type_id
  )
  AS "Min",
  (
    SELECT MAX(EX.enum_name) FROM type_enum_lst EX WHERE EX.model_id = M.model_id AND EX.mod_type_id = M.mod_type_id
  )
  AS "Max",
  MT.lang_id      AS "LanguageID"
FROM type_dic M
INNER JOIN type_dic_txt MT ON (MT.model_id = M.model_id AND MT.mod_type_id = M.mod_type_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.dic_id = 3;

CREATE OR REPLACE VIEW RangeValueDic AS
SELECT
  M.mod_type_id   AS "TypeID", 
  E.enum_name     AS "Value"
FROM type_dic M
INNER JOIN type_enum_lst E ON (E.model_id = M.model_id AND E.mod_type_id = M.mod_type_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.dic_id = 3;

CREATE OR REPLACE VIEW PartitionDic AS
SELECT
  M.mod_type_id   AS "TypeID", 
  M.mod_type_name AS "Name",
  MT.descr        AS "Description",
  MT.note         AS "Note",
  M.total_enum_id AS "NumberOfValues",
  MT.lang_id      AS "LanguageID"
FROM type_dic M
INNER JOIN type_dic_txt MT ON (MT.model_id = M.model_id AND MT.mod_type_id = M.mod_type_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.dic_id = 4;

CREATE OR REPLACE VIEW PartitionValueDic AS
SELECT
  M.mod_type_id   AS "TypeID", 
  E.enum_id       AS "Position",
  E.enum_name     AS "Value",
  E.enum_name     AS "StringValue"
FROM type_dic M
INNER JOIN type_enum_lst E ON (E.model_id = M.model_id AND E.mod_type_id = M.mod_type_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.dic_id = 4;

CREATE OR REPLACE VIEW PartitionIntervalDic AS
SELECT
  M.mod_type_id   AS "TypeID", 
  E.enum_id       AS "Position",
  ET.descr        AS "Description",
  ET.lang_id      AS "LanguageID"
FROM type_dic M
INNER JOIN type_enum_lst E ON (E.model_id = M.model_id AND E.mod_type_id = M.mod_type_id)
INNER JOIN type_enum_txt ET ON (ET.model_id = E.model_id AND ET.mod_type_id = E.mod_type_id AND ET.enum_id = E.enum_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.dic_id = 4;

-- CREATE OR REPLACE VIEW LinkTypeDic AS
-- SELECT
--   M.mod_type_id   AS "TypeID", 
--   M.mod_type_name AS "Name",
--   NULL            AS "LinkedActorID"
-- FROM type_dic M
-- WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
-- AND M.dic_id = 5;

CREATE OR REPLACE VIEW ParameterDic AS
SELECT
  M.parameter_id   AS "ParameterID", 
  M.parameter_name AS "Name", 
  DT.descr         AS "Description",
  DT.note          AS "Note",
  RT.note          AS "ValueNote",
  M.mod_type_id    AS "TypeID", 
  M.parameter_rank AS "Rank",
  M.num_cumulated  AS "NumberOfCumulatedDimensions",
  M.is_generated   AS "ModelGenerated",
  M.is_hidden      AS "Hidden",
  DT.lang_id       AS "LanguageID" 
FROM parameter_dic M
INNER JOIN parameter_dic_txt DT ON (DT.model_id = M.model_id AND DT.parameter_id = M.parameter_id)
LEFT OUTER JOIN parameter_run_txt RT ON 
  (
    RT.model_id = DT.model_id AND RT.parameter_id = DT.parameter_id AND RT.lang_id = DT.lang_id
    AND RT.run_id = 
    (
      SELECT MIN(MR.run_id) FROM run_lst MR WHERE MR.model_id = M.model_id
    )
  )
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE OR REPLACE VIEW ParameterDimensionDic AS
SELECT
  M.parameter_id AS "ParameterID", 
  D.dim_pos      AS "DisplayPosition", 
  D.mod_type_id  AS "TypeID", 
  D.dim_pos      AS "Position"
FROM parameter_dic M
INNER JOIN parameter_dims D ON (D.model_id = M.model_id AND D.parameter_id = M.parameter_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE OR REPLACE VIEW ParameterGroupDic AS
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

CREATE OR REPLACE VIEW ParameterGroupMemberDic AS
SELECT
  M.group_id        AS "ParameterGroupID", 
  PC.child_pos      AS "Position", 
  PC.leaf_id        AS "ParameterID",
  PC.child_group_id AS "MemberGroupID" 
FROM group_lst M
INNER JOIN group_pc PC ON (PC.model_id = M.model_id AND PC.group_id = M.group_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_parameter <> 0;

CREATE OR REPLACE VIEW TableDic AS
SELECT
  M.table_id       AS "TableID", 
  M.table_name     AS "Name", 
  T.descr          AS "Description",
  T.note           AS "Note",
  M.table_rank + 1 AS "Rank",
  M.unit_dim_pos   AS "AnalysisDimensionPosition", 
  M.table_name || '.DimA' AS "AnalysisDimensionName", 
  T.unit_descr     AS "AnalysisDimensionDescription",
  T.unit_note      AS "AnalysisDimensionNote",
  M.is_sparse      AS "Sparse",
  M.is_hidden      AS "Hidden",
  T.lang_id        AS "LanguageID" 
FROM table_dic M
INNER JOIN table_dic_txt T ON (T.model_id = M.model_id AND T.table_id = M.table_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_user = 0;

CREATE OR REPLACE VIEW UserTableDic AS
SELECT
  M.table_id       AS "TableID", 
  M.table_name     AS "Name", 
  T.descr          AS "Description",
  T.note           AS "Note",
  M.table_rank + 1 AS "Rank",
  M.unit_dim_pos   AS "AnalysisDimensionPosition", 
  M.table_name || '.DimA' AS "AnalysisDimensionName", 
  T.unit_descr     AS "AnalysisDimensionDescription",
  T.unit_note      AS "AnalysisDimensionNote",
  M.is_sparse      AS "Sparse",
  M.is_hidden      AS "Hidden",
  T.lang_id        AS "LanguageID" 
FROM table_dic M
INNER JOIN table_dic_txt T ON (T.model_id = M.model_id AND T.table_id = M.table_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_user <> 0;

CREATE OR REPLACE VIEW TableClassDic AS
SELECT
  M.table_id    AS "TableID", 
  M.dim_pos     AS "Position",
  TD.table_name || '.' || M.dim_name AS "Name", 
  T.descr       AS "Description",
  T.note        AS "Note",
  M.mod_type_id AS "TypeID", 
  M.is_total    AS "Totals",
  T.lang_id     AS "LanguageID" 
FROM table_dims M
INNER JOIN table_dic TD ON (TD.model_id = M.model_id AND TD.table_id = M.table_id)
INNER JOIN table_dims_txt T ON (T.model_id = M.model_id AND T.table_id = M.table_id AND T.dim_name = M.dim_name)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE OR REPLACE VIEW TableExpressionDic AS
SELECT
  M.table_id      AS "TableID", 
  M.unit_id       AS "ExpressionID", 
  TD.table_name || '.' || M.unit_name AS "Name", 
  T.descr         AS "Description",
  T.note          AS "Note",
  M.unit_decimals AS "Decimals",
  T.lang_id
FROM table_unit M
INNER JOIN table_dic TD ON (TD.model_id = M.model_id AND TD.table_id = M.table_id)
INNER JOIN table_unit_txt T ON (T.model_id = M.model_id AND T.table_id = M.table_id AND T.unit_id = M.unit_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM);

CREATE OR REPLACE VIEW TableGroupDic AS
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

CREATE OR REPLACE VIEW TableGroupMemberDic AS
SELECT
  M.group_id        AS "TableGroupID", 
  PC.child_pos      AS "Position", 
  PC.leaf_id        AS "TableID",
  PC.child_group_id AS "MemberGroupID" 
FROM group_lst M
INNER JOIN group_pc PC ON (PC.model_id = M.model_id AND PC.group_id = M.group_id)
WHERE M.model_id = (SELECT MIN(FM.model_id) FROM model_dic FM)
AND M.is_parameter = 0;
