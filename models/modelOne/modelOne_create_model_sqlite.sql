--
-- Copyright (c) 2013-2015 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--
-- keep dummy sql below to prevent sqlite3 failure due to UTF-8 BOM
-- it is typical problem if .sql saved by Windows text editors
--
SELECT * FROM sqlite_master WHERE 0 = 1;

--
-- list of ids: values < 10 reserved for development and testing
--   model id = 1 is modelOne
--   set id = 2 is default set of input parameters for modelOne
--   set id = 3, 4 modified working sets of input parameters
--

--
-- modelOne metadata
-- model digest: not a real digest (32 digits hex)
--
INSERT INTO model_dic
  (model_id, model_name, model_digest, model_type, model_ver, create_dt, parameter_prefix, workset_prefix, acc_prefix, value_prefix)
VALUES
  (1, 'modelOne', '20120817_1604590148', 0, '1.0', '2012-08-17 16:04:59.0148', '_p', '_w', '_a', '_v');
  
INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (1, 0, 'First model', NULL);
INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (1, 1, 'First model (fr)', NULL);

-- 
-- modelOne simple types: reference to built-in types
--
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 3, 3);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 13, 13);

-- 
-- modelOne classifcation types
-- type digest: not a real digest (32 digits hex)
--
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (26, 'age', '2012817_160459_0121', 2, 4);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (27, 'sex', '2012817_160459_0122', 2, 2);
INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id) VALUES (28, 'salary', '2012817_160459_0123', 2, 3);

INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 31, 26);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 32, 27);
INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1, 33, 28);

INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (26, 0, 'Age', NULL);
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (26, 1, 'Age (fr)', NULL);
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (27, 0, 'Sex', NULL);
INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (28, 0, 'Salary', NULL);

INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (26, 0, '10');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (26, 1, '20');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (26, 2, '30');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (26, 3, '40');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (27, 0, 'M');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (27, 1, 'F');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (28, 0, 'L');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (28, 1, 'M');
INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (28, 2, 'H');

INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (26, 0, 0, 'age 10', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (26, 0, 1, 'age 10 (fr)', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (26, 1, 0, 'age 20', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (26, 1, 1, 'age 20 (fr)', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (26, 2, 0, 'age 30', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (26, 2, 1, 'age 30 (fr)', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (26, 3, 0, 'age 40', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (26, 3, 1, 'age 40 (fr)', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (27, 0, 0, 'Male', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (27, 1, 0, 'Female', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (28, 0, 0, 'Low', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (28, 1, 0, 'Medium', NULL);
INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (28, 2, 0, 'High', NULL);

-- 
-- modelOne input parameters
-- parameter digest: not a real digest (32 digits hex)
-- db suffix: not a real value (8 digits hex)
--
INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, db_prefix, db_suffix, parameter_rank, type_hid, num_cumulated)
VALUES
  (4, 'ageSex', '2012_817_1604590131', 'ageSex', '2012_817', 2, 13, 0);

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, db_prefix, db_suffix, parameter_rank, type_hid, num_cumulated)
VALUES
  (5, 'salaryAge', '2012_817_1604590132', 'salaryAge', '2012_818', 2, 3, 0);

INSERT INTO parameter_dic
  (parameter_hid, parameter_name, parameter_digest, db_prefix, db_suffix, parameter_rank, type_hid, num_cumulated)
VALUES
  (6, 'StartingSeed', '2012_817_1604590133', 'StartingSeed', '2012_819', 0, 3, 0);

INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden, is_generated) VALUES (1, 0, 4, 0, 0);
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden, is_generated) VALUES (1, 1, 5, 0, 0);
INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden, is_generated) VALUES (1, 2, 6, 0, 0);

INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (4, 0, 'Age by Sex', 'Age by Sex note');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (4, 1, 'Age by Sex (fr)', NULL);
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (5, 0, 'Salary by Age', 'Salary by Age note');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (5, 1, 'Salary by Age (fr)', 'Salary by Age note (fr)');
INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (6, 1, 'Starting Seed', 'Random numbers generator starting seed value');

INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (4, 0, 'dim0', 26);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (4, 1, 'dim1', 27);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (5, 0, 'dim0', 28);
INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (5, 1, 'dim1', 26);

INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (4, 0, 0, 'Age Dim', 'Age Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (4, 0, 1, 'Age Dim (fr)', 'Age Dim notes (fr)');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (4, 1, 0, 'Sex Dim', 'Sex Dim notes');
INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (4, 1, 1, 'Sex Dim', NULL);

-- 
-- modelOne output tables
-- output table digest: not a real digest (32 digits hex)
-- db suffix: not a real value (8 digits hex)
--
INSERT INTO table_dic 
  (table_hid, table_name, table_digest, db_prefix, db_suffix, table_rank, is_sparse) 
VALUES 
  (2, 'salarySex', '2012817_16_04590141', 'salarySex', '2012_820', 2, 1);

INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos) VALUES (1, 0, 2, 0, 1);
  
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (2, 0, 'Salary by Sex', 'Salary by Sex notes', 'Salary Dim', NULL);
INSERT INTO table_dic_txt
  (table_hid, lang_id, descr, note, expr_descr, expr_note)
VALUES
  (2, 1, 'Salary by Sex (fr)', 'Salary by Sex notes (fr)', 'Salary Dim (fr)', NULL);

INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (2, 0, 'dim0', 28, 0, 3);
INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (2, 1, 'dim1', 27, 0, 2);

INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (2, 0, 0, 'Salary Dim', 'Salary Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (2, 0, 1, 'Salary Dim (fr)', 'Salary Dim notes (fr)');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (2, 1, 0, 'Sex Dim', 'Sex Dim notes');
INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (2, 1, 1, 'Sex Dim (fr)', NULL);

INSERT INTO table_acc (table_hid, acc_id, acc_name, acc_expr) VALUES (2, 0, 'acc0', 'value_sum()');
INSERT INTO table_acc (table_hid, acc_id, acc_name, acc_expr) VALUES (2, 1, 'acc1', 'value_count()');

INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (2, 0, 0, 'Sum of salary by sex', NULL);
INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (2, 1, 0, 'Count of salary by sex', NULL);

INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (2, 0, 'expr0', 4, 'OM_AVG(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc_value) AS expr0 FROM salarySex_a2012_820 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (2, 1, 'expr1', 4, 'OM_SUM(acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, SUM(M1.acc_value) AS expr1 FROM salarySex_a2012_820 M1 WHERE M1.acc_id = 1 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (2, 2, 'expr2', 2, 'OM_MIN(acc0)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, MIN(M1.acc_value) AS expr2 FROM salarySex_a2012_820 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
  );
INSERT INTO table_expr 
  (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) 
VALUES 
  (2, 3, 'expr3', 3, 'OM_AVG(acc0 * acc1)', 
  'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc_value * A1.acc1) AS expr3 FROM salarySex_a2012_820 M1 INNER JOIN (SELECT run_id, dim0, dim1, sub_id, acc_value AS acc1 FROM salarySex_a2012_820 WHERE acc_id = 1) A1 ON (A1.run_id = M1.run_id AND A1.dim0 = M1.dim0 AND A1.dim1 = M1.dim1 AND A1.sub_id = M1.sub_id) WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
);
  
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (2, 0, 0, 'Average acc0', 'Average on acc0 notes');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (2, 0, 1, 'Average acc0 (fr)', 'Average on acc0 notes (fr)');
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (2, 1, 0, 'Sum acc1', NULL);
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (2, 2, 0, 'Min acc0', NULL);
INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (2, 3, 0, 'Average acc0 * acc1', NULL);

--
-- modelOne input parameters
--
CREATE TABLE ageSex_p2012_817 
(
  run_id      INT   NOT NULL,
  dim0        INT   NOT NULL, 
  dim1        INT   NOT NULL, 
  param_value FLOAT NOT NULL,
  PRIMARY KEY (run_id, dim0, dim1)
);

CREATE TABLE ageSex_w2012_817
(
  set_id      INT   NOT NULL,
  dim0        INT   NOT NULL, 
  dim1        INT   NOT NULL, 
  param_value FLOAT NOT NULL,
  PRIMARY KEY (set_id, dim0, dim1)
);

CREATE TABLE salaryAge_p2012_818
(
  run_id      INT NOT NULL,
  dim0        INT NOT NULL, 
  dim1        INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (run_id, dim0, dim1)
);

CREATE TABLE salaryAge_w2012_818
(
  set_id      INT NOT NULL,
  dim0        INT NOT NULL, 
  dim1        INT NOT NULL, 
  param_value INT NOT NULL,
  PRIMARY KEY (set_id, dim0, dim1)
);

CREATE TABLE StartingSeed_p2012_819
(
  run_id      INT NOT NULL,
  param_value INT NOT NULL,
  PRIMARY KEY (run_id)
);

CREATE TABLE StartingSeed_w2012_819
(
  set_id      INT NOT NULL,
  param_value INT NOT NULL,
  PRIMARY KEY (set_id)
);

--
-- modelOne output tables
--
CREATE TABLE salarySex_a2012_820
(
  run_id    INT   NOT NULL,
  dim0      INT   NOT NULL, 
  dim1      INT   NOT NULL, 
  acc_id    INT   NOT NULL, 
  sub_id    INT   NOT NULL, 
  acc_value FLOAT NULL,
  PRIMARY KEY (run_id, dim0, dim1, acc_id, sub_id)
);

CREATE TABLE salarySex_v2012_820
(
  run_id     INT   NOT NULL,
  dim0       INT   NOT NULL, 
  dim1       INT   NOT NULL, 
  expr_id    INT   NOT NULL, 
  expr_value FLOAT NULL,
  PRIMARY KEY (run_id, dim0, dim1, expr_id)
);
