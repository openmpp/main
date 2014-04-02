--
-- Copyright (c) 2013 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--
-- keep dummy sql below to prevent sqlite3 failure due to UTF-8 BOM
-- it is typical problem if .sql saved by Windows text editors
--
SELECT * FROM sqlite_master WHERE 0 = 1;

--
-- list of ids: values < 10 reserved for development and testing
-- test: model id = 1 is ModelOne
-- test: set id = 2 is default set of parameters for ModelOne
-- test: set id = 3 is modified working set for ModelOne
--
-- INSERT INTO id_lst (id_key, id_value) VALUES ('model_id', 10);
-- INSERT INTO id_lst (id_key, id_value) VALUES ('run_set', 10);

--
-- ModelOne metadata: describe the model
--
INSERT INTO model_dic
  (model_id, model_name, model_type, model_ver, model_ts, model_prefix, parameter_prefix, workset_prefix, sub_prefix, value_prefix)
VALUES
  (1, 'modelOne', 0, '0.1', '_201208171604590148_', 'modelone_201208171604590148_', 'p', 'w', 's', 'v');

INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (1, 0, 'First model', NULL);
INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (1, 1, 'First model (fr)', NULL);

-- 
-- ModelOne default profile: default run options
--
INSERT INTO profile_lst (profile_name) VALUES ('ModelOne');

INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('ModelOne', 'Parameter.StartingSeed', '4095');
INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('ModelOne', 'OpenM.SparseOutput', 'true');
-- INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('ModelOne', 'OpenM.SparseNullValue', '');
-- INSERT INTO profile_option (profile_name, option_key, option_value) VALUES ('ModelOne', 'General.Subsamples', '1');

-- 
-- ModelOne simple types
--
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 0,  'int', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 1,  'char', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 2,  'short', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 3,  'long', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 4,  'uint', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 5,  'uchar', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 6,  'ushort', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 7,  'ulong', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 8,  'integer', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 9,  'counter', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 10, 'real', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 11, 'float', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 12, 'double', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 13, 'TIME', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 14, 'rate', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 15, 'cumrate', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 16, 'haz1rate', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 17, 'haz2rate', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 18, 'piece_linear', 0, 1);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 19, 'file', 0, 1);

-- 
-- ModelOne logical type
--
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 20, 'logical', 1, 2);

INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 20, 0, 'logical type', NULL);
INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 20, 1, 'logical type (fr)', NULL);

INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 20, 0, 'FALSE');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 20, 1, 'TRUE');

INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 20, 0, 0, 'False', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 20, 0, 1, 'Faux', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 20, 1, 0, 'True', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 20, 1, 1, 'Vrai', NULL);

-- 
-- ModelOne classifcation types
--
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 21, 'age', 2, 4);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 22, 'sex', 2, 2);
INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id) VALUES (1, 23, 'salary', 2, 3);

INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 21, 0, 'Age', NULL);
INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 21, 1, 'Age (fr)', NULL);
INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 22, 0, 'Sex', NULL);
INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note) VALUES (1, 23, 0, 'Salary', NULL);

INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 21, 0, '10');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 21, 1, '20');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 21, 2, '30');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 21, 3, '40');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 22, 0, 'M');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 22, 1, 'F');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 23, 0, 'L');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 23, 1, 'M');
INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name) VALUES (1, 23, 2, 'H');

INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 0, 0, 'age 10', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 0, 1, 'age 10 (fr)', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 1, 0, 'age 20', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 1, 1, 'age 20 (fr)', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 2, 0, 'age 30', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 2, 1, 'age 30 (fr)', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 3, 0, 'age 40', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 21, 3, 1, 'age 40 (fr)', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 22, 0, 0, 'Male', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 22, 1, 0, 'Female', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 23, 0, 0, 'Low', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 23, 1, 0, 'Medium', NULL);
INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note) VALUES (1, 23, 2, 0, 'High', NULL);

-- 
-- ModelOne input parameters
--
INSERT INTO parameter_dic
  (model_id, parameter_id, db_name_suffix, parameter_name, parameter_rank, mod_type_id, is_hidden, is_generated, num_cumulated)
VALUES
  (1, 0, '0_ageSex', 'ageSex', 2, 12, 0, 0, 0);
INSERT INTO parameter_dic
  (model_id, parameter_id, db_name_suffix, parameter_name, parameter_rank, mod_type_id, is_hidden, is_generated, num_cumulated)
VALUES
  (1, 1, '1_salaryAge', 'salaryAge', 2, 0, 0, 0, 0);
INSERT INTO parameter_dic
  (model_id, parameter_id, db_name_suffix, parameter_name, parameter_rank, mod_type_id, is_hidden, is_generated, num_cumulated)
VALUES
  (1, 2, '2_StartingSeed', 'StartingSeed', 0, 0, 0, 0, 0);

INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note) VALUES (1, 0, 0, 'Age by Sex', 'Age by Sex note');
INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note) VALUES (1, 0, 1, 'Age by Sex (fr)', NULL);
INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note) VALUES (1, 1, 0, 'Salary by Age', 'Salary by Age note');
INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note) VALUES (1, 1, 1, 'Salary by Age (fr)', 'Salary by Age note (fr)');
INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note) VALUES (1, 2, 1, 'Starting Seed', 'Random numbers generator starting seed value');

INSERT INTO parameter_dims (model_id, parameter_id, dim_name, dim_pos, mod_type_id) VALUES (1, 0, 'dim0', 0, 21);
INSERT INTO parameter_dims (model_id, parameter_id, dim_name, dim_pos, mod_type_id) VALUES (1, 0, 'dim1', 1, 22);
INSERT INTO parameter_dims (model_id, parameter_id, dim_name, dim_pos, mod_type_id) VALUES (1, 1, 'dim0', 0, 23);
INSERT INTO parameter_dims (model_id, parameter_id, dim_name, dim_pos, mod_type_id) VALUES (1, 1, 'dim1', 1, 21);

-- 
-- ModelOne output tables
--
INSERT INTO table_dic 
  (model_id, table_id, db_name_suffix, table_name, is_user, table_rank, is_sparse, is_hidden) 
VALUES 
  (1, 0, '0_salarySex', 'salarySex', 0, 2, 1, 0);

INSERT INTO table_dic_txt
  (model_id, table_id, lang_id, descr, note, unit_descr, unit_note)
VALUES
  (1, 0, 0, 'Salary by Sex', 'Salary by Sex notes', 'Salary Dim', NULL);
INSERT INTO table_dic_txt
  (model_id, table_id, lang_id, descr, note, unit_descr, unit_note)
VALUES
  (1, 0, 1, 'Salary by Sex (fr)', 'Salary by Sex notes (fr)', 'Salary Dim (fr)', NULL);

INSERT INTO table_dims (model_id, table_id, dim_name, dim_pos, mod_type_id, is_total, dim_size) VALUES (1, 0, 'dim0', 0, 23, 0, 3);
INSERT INTO table_dims (model_id, table_id, dim_name, dim_pos, mod_type_id, is_total, dim_size) VALUES (1, 0, 'dim1', 1, 22, 0, 2);

INSERT INTO table_dims_txt (model_id, table_id, dim_name, lang_id, descr, note) VALUES (1, 0, 'dim0', 0, 'Salary Dim', 'Salary Dim notes');
INSERT INTO table_dims_txt (model_id, table_id, dim_name, lang_id, descr, note) VALUES (1, 0, 'dim0', 1, 'Salary Dim (fr)', 'Salary Dim notes (fr)');
INSERT INTO table_dims_txt (model_id, table_id, dim_name, lang_id, descr, note) VALUES (1, 0, 'dim1', 0, 'Sex Dim', 'Sex Dim notes');
INSERT INTO table_dims_txt (model_id, table_id, dim_name, lang_id, descr, note) VALUES (1, 0, 'dim1', 1, 'Sex Dim (fr)', NULL);

INSERT INTO table_acc (model_id, table_id, acc_id, acc_name, acc_expr) VALUES (1, 0, 0, 'acc0', 'value_sum()');
INSERT INTO table_acc (model_id, table_id, acc_id, acc_name, acc_expr) VALUES (1, 0, 1, 'acc1', 'value_count()');

INSERT INTO table_acc_txt (model_id, table_id, acc_id, lang_id, descr, note) VALUES (1, 0, 0, 0, 'Sum of salary by sex', NULL);
INSERT INTO table_acc_txt (model_id, table_id, acc_id, lang_id, descr, note) VALUES (1, 0, 1, 0, 'Count of salary by sex', NULL);

INSERT INTO table_unit 
  (model_id, table_id, unit_id, unit_name, unit_decimals, unit_src, unit_expr) 
VALUES 
  (1, 0, 0, 'Expr0', 4, 'OM_AVG(acc0)', 'AVG(S.acc0)');
INSERT INTO table_unit 
  (model_id, table_id, unit_id, unit_name, unit_decimals, unit_src, unit_expr) 
VALUES 
  (1, 0, 1, 'Expr1', 4, 'OM_SUM(acc1)', 'SUM(S.acc1)');
INSERT INTO table_unit 
  (model_id, table_id, unit_id, unit_name, unit_decimals, unit_src, unit_expr) 
VALUES 
  (1, 0, 2, 'Expr2', 2, 'OM_MIN(acc0)', 'MIN(S.acc0)');
INSERT INTO table_unit 
  (model_id, table_id, unit_id, unit_name, unit_decimals, unit_src, unit_expr) 
VALUES 
  (1, 0, 3, 'Expr3', 3, 'OM_AVG(acc0 * acc1)', 'AVG(S.acc0 * S.acc1)');
  
INSERT INTO table_unit_txt (model_id, table_id, unit_id, lang_id, descr, note) VALUES (1, 0, 0, 0, 'Average acc0', 'Average on acc0 notes');
INSERT INTO table_unit_txt (model_id, table_id, unit_id, lang_id, descr, note) VALUES (1, 0, 0, 1, 'Average acc0 (fr)', 'Average on acc0 notes (fr)');
INSERT INTO table_unit_txt (model_id, table_id, unit_id, lang_id, descr, note) VALUES (1, 0, 1, 0, 'Sum acc1', NULL);
INSERT INTO table_unit_txt (model_id, table_id, unit_id, lang_id, descr, note) VALUES (1, 0, 2, 0, 'Min acc0', NULL);
INSERT INTO table_unit_txt (model_id, table_id, unit_id, lang_id, descr, note) VALUES (1, 0, 3, 0, 'Average acc0 * acc1', NULL);

--
-- ModelOne input parameters
--
CREATE TABLE modelone_201208171604590148_p0_ageSex 
(
  run_id INT   NOT NULL,
  dim0   INT   NOT NULL, 
  dim1   INT   NOT NULL, 
  value  FLOAT NOT NULL,
  PRIMARY KEY (run_id, dim0, dim1)
);

CREATE TABLE modelone_201208171604590148_w0_ageSex
(
  set_id INT   NOT NULL,
  dim0   INT   NOT NULL, 
  dim1   INT   NOT NULL, 
  value  FLOAT NOT NULL,
  PRIMARY KEY (set_id, dim0, dim1)
);

CREATE TABLE modelone_201208171604590148_p1_salaryAge 
(
  run_id INT NOT NULL,
  dim0   INT NOT NULL, 
  dim1   INT NOT NULL, 
  value  INT NOT NULL,
  PRIMARY KEY (run_id, dim0, dim1)
);

CREATE TABLE modelone_201208171604590148_w1_salaryAge
(
  set_id INT NOT NULL,
  dim0   INT NOT NULL, 
  dim1   INT NOT NULL, 
  value  INT NOT NULL,
  PRIMARY KEY (set_id, dim0, dim1)
);

CREATE TABLE modelone_201208171604590148_p2_StartingSeed 
(
  run_id INT NOT NULL,
  value  INT NOT NULL,
  PRIMARY KEY (run_id)
);

CREATE TABLE modelone_201208171604590148_w2_StartingSeed
(
  set_id INT NOT NULL,
  value  INT NOT NULL,
  PRIMARY KEY (set_id)
);

--
-- ModelOne output tables
--
CREATE TABLE modelone_201208171604590148_s0_salarySex
(
  run_id    INT   NOT NULL,
  dim0      INT   NOT NULL, 
  dim1      INT   NOT NULL, 
  sub_id    INT   NOT NULL, 
  acc0      FLOAT NULL,
  acc1      FLOAT NULL,
  PRIMARY KEY (run_id, dim0, dim1, sub_id)
);

CREATE TABLE modelone_201208171604590148_v0_salarySex
(
  run_id    INT   NOT NULL,
  dim0      INT   NOT NULL, 
  dim1      INT   NOT NULL, 
  unit_id   INT   NOT NULL, 
  value     FLOAT NULL,
  PRIMARY KEY (run_id, dim0, dim1, unit_id)
);
