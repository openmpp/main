--
-- Copyright (c) 2014 OpenM++
-- This code is licensed under MIT license (see LICENSE.txt for details)
--
-- keep dummy sql below to prevent sqlite3 failure due to UTF-8 BOM
-- it is typical problem if .sql saved by Windows text editors
--
SELECT * FROM sqlite_master WHERE 0 = 1;

--
-- Id's source table
--
CREATE TABLE id_lst
(
  id_key   VARCHAR(32) NOT NULL, -- id key (based on table)
  id_value INT         NOT NULL, -- current id value
  PRIMARY KEY (id_key)
);

-- 
-- Language list
-- 
CREATE TABLE lang_lst
(
  lang_id   INT          NOT NULL, -- unique language id across all models
  lang_code VARCHAR(32)  NOT NULL, -- language code: en_US
  lang_name VARCHAR(255) NOT NULL, -- language name
  PRIMARY KEY (lang_id),
  CONSTRAINT lang_un UNIQUE (lang_code)
);

-- 
-- Language: list of keywords in specific language: all, min, max
-- 
CREATE TABLE lang_word
(
  lang_id    INT          NOT NULL, -- master key
  word_code  VARCHAR(255) NOT NULL, -- word key: all, min, max
  word_value VARCHAR(255) NOT NULL, -- actual word in that language
  PRIMARY KEY (lang_id, word_code),
  CONSTRAINT lang_word_mk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Model(s) list
--
CREATE TABLE model_dic 
(
  model_id         INT          NOT NULL, -- unique model id for each name and version
  model_name       VARCHAR(255) NOT NULL, -- model name: modelOne
  model_type       INT          NOT NULL, -- model type: 0 = case based, 1 = time based
  model_ver        VARCHAR(255) NOT NULL, -- model version
  model_ts         VARCHAR(32)  NOT NULL, -- compilation timestamp: _201208171604590148_
  model_prefix     VARCHAR(32)  NOT NULL, -- table name prefix: modelOne_201208171604590148_
  parameter_prefix VARCHAR(4)   NOT NULL, -- parameter tables prefix: p
  workset_prefix   VARCHAR(4)   NOT NULL, -- workset tables prefix: w
  acc_prefix       VARCHAR(4)   NOT NULL, -- accumulator tables prefix: a
  value_prefix     VARCHAR(4)   NOT NULL, -- value tables prefix: v
  PRIMARY KEY (model_id), 
  CONSTRAINT model_dic_un_1 UNIQUE (model_name, model_ts),
  CONSTRAINT model_dic_un_2 UNIQUE (model_prefix)
);

--
-- Model(s) list text info
--
CREATE TABLE model_dic_txt 
(
  model_id INT             NOT NULL, -- master key
  lang_id  INT             NOT NULL, -- language id
  descr    VARCHAR(255)    NOT NULL, -- model description
  note     VARCHAR(32000),           -- model notes
  PRIMARY KEY (model_id, lang_id),
  CONSTRAINT model_dic_txt_mk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id),
  CONSTRAINT model_dic_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Model run's list
-- Run id must be different from working set id (use id_lst to get it)
-- Model run is completed (all data saved in database) when sub_completed = sub_count
--
CREATE TABLE run_lst
(
  run_id        INT          NOT NULL, -- unique run id
  model_id      INT          NOT NULL, -- model id
  run_name      VARCHAR(255) NOT NULL, -- model run name
  sub_count     INT          NOT NULL, -- subsamples count
  sub_started   INT          NOT NULL, -- number of subsamples started
  sub_completed INT          NOT NULL, -- number of subsamples completed
  sub_restart   INT          NOT NULL, -- subsample to restart from
  create_dt     VARCHAR(32)  NOT NULL, -- start date-time
  status        VARCHAR(1)   NOT NULL, -- run status: i=init p=progress s=success x=exit e=error(failed)
  update_dt     VARCHAR(32)  NOT NULL, -- last update date-time
  PRIMARY KEY (run_id),
  CONSTRAINT run_lst_mk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Model run's text results
--
CREATE TABLE run_txt
(
  run_id   INT             NOT NULL, -- master key
  model_id INT             NOT NULL, -- model id
  lang_id  INT             NOT NULL, -- language id
  descr    VARCHAR(255)    NOT NULL, -- model run description
  note     VARCHAR(32000),           -- model run notes
  PRIMARY KEY (run_id, lang_id),
  CONSTRAINT run_txt_mk 
             FOREIGN KEY (run_id) REFERENCES run_lst(run_id),
  CONSTRAINT run_txt_lang 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

-- 
-- Types dictionary for model parameters
-- 
CREATE TABLE type_dic
(
  model_id      INT          NOT NULL, -- master key
  mod_type_id   INT          NOT NULL, -- unique type id
  mod_type_name VARCHAR(255) NOT NULL, -- type name: int, double...
  dic_id        INT          NOT NULL, -- dictionary id: 0=simple 1=logical 2=classification 3=range 4=partition 5=link
  total_enum_id INT          NOT NULL, -- if total enabled this is enum_id of "Total" item =max+1
  PRIMARY KEY (model_id, mod_type_id),
  CONSTRAINT type_dic_un UNIQUE (model_id, mod_type_name),
  CONSTRAINT type_dic_mk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

-- 
-- Types dictionary text: description and notes for the type
-- 
CREATE TABLE type_dic_txt
(
  model_id    INT             NOT NULL, -- master key
  mod_type_id INT             NOT NULL, -- master key
  lang_id     INT             NOT NULL, -- language id
  descr       VARCHAR(255)    NOT NULL, -- type description
  note        VARCHAR(32000),           -- type notes
  PRIMARY KEY (model_id, mod_type_id, lang_id),
  CONSTRAINT type_dic_txt_mk 
             FOREIGN KEY (model_id, mod_type_id) REFERENCES type_dic (model_id, mod_type_id),
  CONSTRAINT type_dic_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- List of values for enum types, i.e. enum values for classification dictionary
--
CREATE TABLE type_enum_lst
(
  model_id    INT          NOT NULL, -- master key
  mod_type_id INT          NOT NULL, -- master key
  enum_id     INT          NOT NULL, -- unique enum id, zero-based
  enum_name   VARCHAR(255) NOT NULL, -- enum name, ie: "true", "Ontario", "[90, max["
  PRIMARY KEY (model_id, mod_type_id, enum_id),
  CONSTRAINT type_enum_lst_mk
             FOREIGN KEY (model_id, mod_type_id) REFERENCES type_dic (model_id, mod_type_id)
);

-- 
-- Enum value text: description and notes for type values
-- 
CREATE TABLE type_enum_txt
(
  model_id    INT             NOT NULL, -- master key
  mod_type_id INT             NOT NULL, -- master key
  enum_id     INT             NOT NULL, -- master key
  lang_id     INT             NOT NULL, -- language
  descr       VARCHAR(255)    NOT NULL, -- value description
  note        VARCHAR(32000),           -- value notes
  PRIMARY KEY (model_id, mod_type_id, enum_id, lang_id),
  CONSTRAINT type_enum_txt_mk 
             FOREIGN KEY (model_id, mod_type_id, enum_id) REFERENCES type_enum_lst (model_id, mod_type_id, enum_id),
  CONSTRAINT type_enum_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Parameter tables dictionary
--
CREATE TABLE parameter_dic
(
  model_id       INT          NOT NULL, -- master key
  parameter_id   INT          NOT NULL, -- unique parameter id
  db_name_suffix VARCHAR(32)  NOT NULL, -- unique part for db table name: 1234_ageSex
  parameter_name VARCHAR(255) NOT NULL, -- parameter name
  parameter_rank INT          NOT NULL, -- parameter rank
  mod_type_id    INT          NOT NULL, -- parameter type id
  is_hidden      SMALLINT     NOT NULL, -- if <> 0 then parameter is hidden
  is_generated   SMALLINT     NOT NULL, -- if <> 0 then parameter is model generated
  num_cumulated  INT          NOT NULL, -- number of cumulated dimensions
  PRIMARY KEY (model_id, parameter_id),
  CONSTRAINT parameter_dic_un UNIQUE (model_id, db_name_suffix),
  CONSTRAINT parameter_dic_mk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id),
  CONSTRAINT parameter_dic_type_fk
             FOREIGN KEY (model_id, mod_type_id) REFERENCES type_dic (model_id, mod_type_id)
);

--
-- Parameter tables text info
--
CREATE TABLE parameter_dic_txt
(
  model_id     INT             NOT NULL, -- master key
  parameter_id INT             NOT NULL, -- master key
  lang_id      INT             NOT NULL, -- language id
  descr        VARCHAR(255)    NOT NULL, -- parameter description
  note         VARCHAR(32000),           -- parameter notes
  PRIMARY KEY (model_id, parameter_id, lang_id),
  CONSTRAINT parameter_dic_txt_mk
             FOREIGN KEY (model_id, parameter_id) REFERENCES parameter_dic (model_id, parameter_id),
  CONSTRAINT parameter_dic_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Parameter run text: parameter run value notes
--
CREATE TABLE parameter_run_txt
(
  run_id       INT NOT NULL,   -- model run id
  model_id     INT NOT NULL,   -- master key
  parameter_id INT NOT NULL,   -- master key
  lang_id      INT NOT NULL,   -- language id
  note         VARCHAR(32000), -- parameter value notes
  PRIMARY KEY (run_id, parameter_id, lang_id),
  CONSTRAINT parameter_run_txt_mk 
             FOREIGN KEY (model_id, parameter_id) REFERENCES parameter_dic (model_id, parameter_id),
  CONSTRAINT parameter_run_txt_run_fk 
             FOREIGN KEY (run_id) REFERENCES run_lst (run_id),
  CONSTRAINT parameter_run_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Parameter dimension(s)
--
CREATE TABLE parameter_dims
(
  model_id     INT        NOT NULL, -- master key
  parameter_id INT        NOT NULL, -- master key
  dim_id       INT        NOT NULL, -- unique dimension id
  dim_name     VARCHAR(8) NOT NULL, -- column name: dim0
  mod_type_id  INT        NOT NULL, -- dimension type id
  PRIMARY KEY (model_id, parameter_id, dim_id),
  CONSTRAINT parameter_dims_un UNIQUE (model_id, parameter_id, dim_name),
  CONSTRAINT parameter_dims_mk
             FOREIGN KEY (model_id, parameter_id) REFERENCES parameter_dic (model_id, parameter_id),
  CONSTRAINT parameter_dims_type_fk
             FOREIGN KEY (model_id, mod_type_id) REFERENCES type_dic (model_id, mod_type_id)
);

--
-- Dimensions text info for parameter
--
CREATE TABLE parameter_dims_txt 
(
  model_id     INT             NOT NULL, -- master key
  parameter_id INT             NOT NULL, -- master key
  dim_id       INT             NOT NULL, -- master key
  lang_id      INT             NOT NULL, -- language id
  descr        VARCHAR(255)    NOT NULL, -- table dimension description
  note         VARCHAR(32000),           -- table dimension notes
  PRIMARY KEY (model_id, parameter_id, dim_id, lang_id),
  CONSTRAINT parameter_dims_txt_mk 
             FOREIGN KEY (model_id, parameter_id, dim_id) REFERENCES parameter_dims (model_id, parameter_id, dim_id),
  CONSTRAINT parameter_dims_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Output result tables
--
CREATE TABLE table_dic 
(
  model_id       INT          NOT NULL, -- master key
  table_id       INT          NOT NULL, -- unique table id
  db_name_suffix VARCHAR(32)  NOT NULL, -- unique part for db table name: 1234_salaryBySex
  table_name     VARCHAR(255) NOT NULL, -- table name
  is_user        SMALLINT     NOT NULL, -- if <> 0 then "user" table
  table_rank     INT          NOT NULL, -- table rank
  is_sparse      SMALLINT     NOT NULL, -- if <> 0 then table stored as sparse
  is_hidden      SMALLINT     NOT NULL, -- if <> 0 then table is hidden
  expr_dim_pos   INT          NOT NULL, -- table expressions dimension (analysis dimension) position
  PRIMARY KEY (model_id, table_id),
  CONSTRAINT table_dic_un UNIQUE (model_id, db_name_suffix),
  CONSTRAINT table_dic_mk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Output result tables text info
--
CREATE TABLE table_dic_txt 
(
  model_id   INT             NOT NULL, -- master key
  table_id   INT             NOT NULL, -- master key
  lang_id    INT             NOT NULL, -- language id
  descr      VARCHAR(255)    NOT NULL, -- table description
  note       VARCHAR(32000),           -- table notes
  expr_descr VARCHAR(255)    NOT NULL, -- table expressions (analysis dimension) description
  expr_note  VARCHAR(32000),           -- table expressions (analysis dimension) notes
  PRIMARY KEY (model_id, table_id, lang_id),
  CONSTRAINT table_dic_txt_mk 
             FOREIGN KEY (model_id, table_id) REFERENCES table_dic (model_id, table_id),
  CONSTRAINT table_dic_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Dimensions for output tables
--
CREATE TABLE table_dims 
(
  model_id    INT        NOT NULL, -- master key
  table_id    INT        NOT NULL, -- master key
  dim_id      INT        NOT NULL, -- unique dimension id
  dim_name    VARCHAR(8) NOT NULL, -- unique column name of dimension: dim0
  mod_type_id INT        NOT NULL, -- dimension type
  is_total    SMALLINT   NOT NULL, -- if <> 0 then dimension has "total" item
  dim_size    INT        NOT NULL, -- number of items, including "total" item
  PRIMARY KEY (model_id, table_id, dim_id),
  CONSTRAINT table_dims_un UNIQUE (model_id, table_id, dim_name),
  CONSTRAINT table_dims_mk 
             FOREIGN KEY (model_id, table_id) REFERENCES table_dic (model_id, table_id),
  CONSTRAINT table_dims_type_fk 
             FOREIGN KEY (model_id, mod_type_id) REFERENCES type_dic (model_id, mod_type_id)
);

--
-- Dimensions text info for output tables
--
CREATE TABLE table_dims_txt 
(
  model_id INT             NOT NULL, -- master key
  table_id INT             NOT NULL, -- master key
  dim_id   INT             NOT NULL, -- master key
  lang_id  INT             NOT NULL, -- language id
  descr    VARCHAR(255)    NOT NULL, -- table dimension description
  note     VARCHAR(32000),           -- table dimension notes
  PRIMARY KEY (model_id, table_id, dim_id, lang_id),
  CONSTRAINT table_dims_txt_mk 
             FOREIGN KEY (model_id, table_id, dim_id) REFERENCES table_dims (model_id, table_id, dim_id),
  CONSTRAINT table_dims_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Output table accumulators
--
CREATE TABLE table_acc 
(
  model_id INT          NOT NULL, -- master key
  table_id INT          NOT NULL, -- master key
  acc_id  INT           NOT NULL, -- unique accumulator id
  acc_name VARCHAR(8)   NOT NULL, -- unique accumulator name: acc2
  acc_expr VARCHAR(255) NOT NULL, -- accumulator expression: min_value_out(duration())
  PRIMARY KEY (model_id, table_id, acc_id),
  CONSTRAINT table_acc_un UNIQUE (model_id, table_id, acc_name),
  CONSTRAINT table_acc_mk 
             FOREIGN KEY (model_id, table_id) REFERENCES table_dic (model_id, table_id)
);

--
-- Output table accumulators text info
--
CREATE TABLE table_acc_txt 
(
  model_id INT          NOT NULL, -- master key
  table_id INT          NOT NULL, -- master key
  acc_id   INT          NOT NULL, -- master key
  lang_id  INT          NOT NULL, -- language id
  descr    VARCHAR(255) NOT NULL, -- item description
  note     VARCHAR(32000),        -- item notes
  PRIMARY KEY (model_id, table_id, acc_id, lang_id),
  CONSTRAINT table_acc_txt_mk 
             FOREIGN KEY (model_id, table_id, acc_id) REFERENCES table_acc (model_id, table_id, acc_id),
  CONSTRAINT table_acc_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Output table expressions (analysis dimension items)
--
CREATE TABLE table_expr
(
  model_id      INT           NOT NULL, -- master key
  table_id      INT           NOT NULL, -- master key
  expr_id       INT           NOT NULL, -- unique item id
  expr_name     VARCHAR(8)    NOT NULL, -- item name: Expr2
  expr_decimals INT           NOT NULL, -- number of decimals for that item
  expr_src      VARCHAR(255)  NOT NULL, -- source expression: OM_AVG(acc3/acc0)
  expr_sql      VARCHAR(2048) NOT NULL, -- db expression: AVG(S.acc3/S.acc0)
  PRIMARY KEY (model_id, table_id, expr_id),
  CONSTRAINT table_expr_un UNIQUE (model_id, table_id, expr_name),
  CONSTRAINT table_expr_mk 
             FOREIGN KEY (model_id, table_id) REFERENCES table_dic (model_id, table_id)
);

--
-- Output table expressions text info
--
CREATE TABLE table_expr_txt 
(
  model_id INT          NOT NULL, -- master key
  table_id INT          NOT NULL, -- master key
  expr_id  INT          NOT NULL, -- master key
  lang_id  INT          NOT NULL, -- language id
  descr    VARCHAR(255) NOT NULL, -- item description
  note     VARCHAR(32000),        -- item notes
  PRIMARY KEY (model_id, table_id, expr_id, lang_id),
  CONSTRAINT table_expr_txt_mk 
             FOREIGN KEY (model_id, table_id, expr_id) REFERENCES table_expr (model_id, table_id, expr_id),
  CONSTRAINT table_expr_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Groups: parameter or output table groups
--
CREATE TABLE group_lst
(
  model_id     INT          NOT NULL, -- master key
  group_id     INT          NOT NULL, -- unique model group id
  is_parameter SMALLINT     NOT NULL, -- if <> 0 then parameter group else output table group
  group_name   VARCHAR(255) NOT NULL, -- group name
  is_hidden    SMALLINT     NOT NULL, -- if <> 0 then group is hidden
  is_generated SMALLINT     NOT NULL, -- if <> 0 then group is model generated
  PRIMARY KEY (model_id, group_id),
  CONSTRAINT group_lst_mk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Group text info for parameter or output table groups
--
CREATE TABLE group_txt
(
  model_id INT             NOT NULL, -- master key
  group_id INT             NOT NULL, -- master key
  lang_id  INT             NOT NULL, -- language id
  descr    VARCHAR(255)    NOT NULL, -- group description
  note     VARCHAR(32000),           -- group notes
  PRIMARY KEY (model_id, group_id, lang_id),
  CONSTRAINT group_txt_mk 
             FOREIGN KEY (model_id, group_id) REFERENCES group_lst (model_id, group_id),
  CONSTRAINT parameter_group_txt_lang_fk 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Groups parent-child: subgroup, parameter or output table
--
CREATE TABLE group_pc
(
  model_id       INT NOT NULL, -- master key
  group_id       INT NOT NULL, -- master key
  child_pos      INT NOT NULL, -- child position in group
  child_group_id INT NULL,     -- if not IS NULL then child group
  leaf_id        INT NULL,     -- if not IS NULL then parameter id or table id
  PRIMARY KEY (model_id, group_id, child_pos),
  CONSTRAINT group_pc_mk 
             FOREIGN KEY (model_id, group_id) REFERENCES group_lst (model_id, group_id)
);

--
-- Profile list
-- profile is a named group of (key, value) options
-- default model options has profile_name = model_name
--
CREATE TABLE profile_lst
(
  profile_name VARCHAR(255) NOT NULL, -- unique profile name
  PRIMARY KEY (profile_name)
);

--
-- Profile options, ie: ini-file options
--
CREATE TABLE profile_option
(
  profile_name VARCHAR(255)  NOT NULL, -- master key
  option_key   VARCHAR(255)  NOT NULL, -- section.key, ie: General.Subsamples
  option_value VARCHAR(2048) NOT NULL, -- option value
  PRIMARY KEY (profile_name, option_key),
  CONSTRAINT profile_option_mk 
             FOREIGN KEY (profile_name) REFERENCES profile_lst(profile_name)
);

--
-- Run options: options used to run the model
-- model options priority:
-- from command line, ini-file, profile_option or default values
--
CREATE TABLE run_option
(
  run_id       INT           NOT NULL, -- master key
  option_key   VARCHAR(255)  NOT NULL, -- section.key, ie: General.Subsamples
  option_value VARCHAR(2048) NOT NULL, -- option value
  PRIMARY KEY (run_id, option_key),
  CONSTRAINT run_option_mk 
             FOREIGN KEY (run_id) REFERENCES run_lst(run_id)
);

--
-- Workset (working set of model parameters):
--   it can be a full set, which include all model parameters 
--   or subset and include only some parameters
-- each model must have "default" workset:
--   default workset must include ALL model parameters (it is a full set)
--   default workset is a first workset of the model: set_id = min(set_id)
-- if workset is a subset (does not include all model parameters)
--   then it must be based on model run results, specified by run_id (not NULL)
-- workset can be editable or read-only
--   if workset is editable then you can modify input parameters or workset description, notes, etc.
--   if workset is read-only then you can run the model using that workset as input
-- Important: working set_id must be different from run_id (use id_lst to get it)
-- Important: always update parameter values inside of transaction scope
-- Important: before parameter update do is_readonly = is_readonly + 1 to "lock" workset
--
CREATE TABLE workset_lst
(
  set_id      INT          NOT NULL, -- unique working set id
  run_id      INT          NULL,     -- if not NULL and positive then base run id (source run id)
  model_id    INT          NOT NULL, -- model id
  set_name    VARCHAR(255) NOT NULL, -- working set name
  is_readonly SMALLINT     NOT NULL, -- if non-zero then working set is read-only
  update_dt   VARCHAR(32)  NOT NULL, -- last update date-time
  PRIMARY KEY (set_id),
  CONSTRAINT workset_lst_mk 
             FOREIGN KEY (run_id) REFERENCES run_lst (run_id)
);

--
-- Working set text: description and notes
--
CREATE TABLE workset_txt
(
  set_id   INT             NOT NULL, -- master key
  model_id INT             NOT NULL, -- model id
  lang_id  INT             NOT NULL, -- language id
  descr    VARCHAR(255)    NOT NULL, -- working set description
  note     VARCHAR(32000),           -- working set notes
  PRIMARY KEY (set_id, lang_id),
  CONSTRAINT workset_txt_mk 
             FOREIGN KEY (set_id) REFERENCES workset_lst (set_id),
  CONSTRAINT workset_txt_lang 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Working set parameters list
--
CREATE TABLE workset_parameter
(
  set_id       INT NOT NULL, -- master key
  model_id     INT NOT NULL, -- model id
  parameter_id INT NOT NULL, -- parameter_dic.parameter_id
  PRIMARY KEY (set_id, parameter_id),
  CONSTRAINT workset_parameter_mk 
             FOREIGN KEY (set_id) REFERENCES workset_lst (set_id),
  CONSTRAINT workset_parameter_param_fk
             FOREIGN KEY (model_id, parameter_id) REFERENCES parameter_dic (model_id, parameter_id)
);

--
-- Working set parameter text: parameter value notes
--
CREATE TABLE workset_parameter_txt
(
  set_id       INT             NOT NULL, -- master key
  model_id     INT             NOT NULL, -- model id
  parameter_id INT             NOT NULL, -- master key
  lang_id      INT             NOT NULL, -- language id
  note         VARCHAR(32000),           -- parameter value note
  PRIMARY KEY (set_id, parameter_id, lang_id),
  CONSTRAINT workset_parameter_txt_mk 
             FOREIGN KEY (set_id, parameter_id) REFERENCES workset_parameter (set_id, parameter_id),
  CONSTRAINT workset_parameter_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Modelling task: named set of model inputs (of working sets)
--
CREATE TABLE task_lst
(
  task_id   INT          NOT NULL, -- unique task id
  model_id  INT          NOT NULL, -- master key
  task_name VARCHAR(255) NOT NULL, -- task name
  PRIMARY KEY (task_id),
  CONSTRAINT task_lst_mk 
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Modelling task text: description and notes
--
CREATE TABLE task_txt
(
  task_id INT             NOT NULL, -- master key
  lang_id INT             NOT NULL, -- language id
  descr   VARCHAR(255)    NOT NULL, -- task description
  note    VARCHAR(32000),           -- task notes
  PRIMARY KEY (task_id, lang_id),
  CONSTRAINT task_txt_mk 
             FOREIGN KEY (task_id) REFERENCES task_lst(task_id),
  CONSTRAINT task_txt_lang 
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Task input: working sets of model input parameters
--
CREATE TABLE task_set
(
  task_id INT NOT NULL, -- master key
  set_id  INT NOT NULL, -- input working set id
  PRIMARY KEY (task_id, set_id),
  CONSTRAINT task_set_mk 
             FOREIGN KEY (task_id) REFERENCES task_lst (task_id),
  CONSTRAINT task_set_fk 
             FOREIGN KEY (set_id) REFERENCES workset_lst (set_id)
);

--
-- Task run history and status
-- if status = w (wait) then 
--    model wait and NOT completed until other process set status to one of finals: s,x,e
--    model check if any new sets inserted into task_set and run it as they arrive
--
CREATE TABLE task_run_lst
(
  task_run_id INT         NOT NULL, -- unique task run id
  task_id     INT         NOT NULL, -- master key
  sub_count   INT         NOT NULL, -- subsamples count of task run
  create_dt   VARCHAR(32) NOT NULL, -- start date-time
  status      VARCHAR(1)  NOT NULL, -- task status: i=init p=progress w=wait s=success x=exit e=error(failed)
  update_dt   VARCHAR(32) NOT NULL, -- last update date-time
  PRIMARY KEY (task_run_id),
  CONSTRAINT task_run_lst_mk 
             FOREIGN KEY (task_id) REFERENCES task_lst (task_id)
);

--
-- Task run: input (working sets) and output (model run)
--
CREATE TABLE task_run_set
(
  task_run_id INT NOT NULL, -- master key
  run_id      INT NOT NULL, -- if > 0 then result run id
  set_id      INT NOT NULL, -- if > 0 then input working set id
  task_id     INT NOT NULL, -- link to task list
  PRIMARY KEY (task_run_id, run_id),
  CONSTRAINT task_rs_un 
             UNIQUE (task_run_id, set_id),
  CONSTRAINT task_rs_mk 
             FOREIGN KEY (task_run_id) REFERENCES task_run_lst (task_run_id)
);

--
-- list of ids, must be positive.
-- values < 10 reserved for development and testing
--
INSERT INTO id_lst (id_key, id_value) VALUES ('lang_id', 10);
INSERT INTO id_lst (id_key, id_value) VALUES ('model_id', 10);
INSERT INTO id_lst (id_key, id_value) VALUES ('run_id_set_id', 10);
INSERT INTO id_lst (id_key, id_value) VALUES ('task_id', 10);
INSERT INTO id_lst (id_key, id_value) VALUES ('task_run_id', 10);

--
-- Languages and word list
--
INSERT INTO lang_lst (lang_id, lang_code, lang_name) VALUES (0, 'EN', 'English');
INSERT INTO lang_lst (lang_id, lang_code, lang_name) VALUES (1, 'FR', 'Français');

INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'all', 'All');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'min', 'min');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (0, 'max', 'max');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'all', 'Toutes');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'min', 'min');
INSERT INTO lang_word (lang_id, word_code, word_value) VALUES (1, 'max', 'max');

