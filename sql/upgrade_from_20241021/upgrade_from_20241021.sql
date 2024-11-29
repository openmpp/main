--
-- upgrade model database
-- from versions [2023-02-01, 2024-10-21] a.k.a. [v1.14.0, v1.17.5] 
--
BEGIN EXCLUSIVE TRANSACTION;

--
-- Groups of entity attributes
--
CREATE TABLE entity_group_lst
(
  model_id        INT          NOT NULL, -- master key
  model_entity_id INT          NOT NULL, -- model entity id
  group_id        INT          NOT NULL, -- unique model group id
  group_name      VARCHAR(255) NOT NULL, -- group name
  is_hidden       SMALLINT     NOT NULL, -- if non-zero then group is hidden
  PRIMARY KEY (model_id, model_entity_id, group_id),
  CONSTRAINT entity_group_lst_un UNIQUE (model_id, model_entity_id, group_name),
  CONSTRAINT entity_group_lst_mk
             FOREIGN KEY (model_id, model_entity_id) REFERENCES model_entity_dic (model_id, model_entity_id)
);

--
-- Group text info for entity attribute groups
--
CREATE TABLE entity_group_txt
(
  model_id        INT             NOT NULL, -- master key
  model_entity_id INT             NOT NULL, -- master key
  group_id        INT             NOT NULL, -- master key
  lang_id         INT             NOT NULL, -- language id
  descr           VARCHAR(255)    NOT NULL, -- group description
  note            VARCHAR(32000),           -- group notes
  PRIMARY KEY (model_id, model_entity_id, group_id, lang_id),
  CONSTRAINT entity_group_txt_mk
             FOREIGN KEY (model_id, model_entity_id, group_id) REFERENCES entity_group_lst (model_id, model_entity_id, group_id),
  CONSTRAINT entity_group_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Entity attribute groups parent-child: subgroup or entity attribute
--
CREATE TABLE entity_group_pc
(
  model_id        INT NOT NULL, -- master key
  model_entity_id INT NOT NULL, -- master key
  group_id        INT NOT NULL, -- master key
  child_pos       INT NOT NULL, -- child position in group
  child_group_id  INT,          -- if not IS NULL then child group
  attr_id         INT,          -- if not IS NULL then entity attribute index
  PRIMARY KEY (model_id, model_entity_id, group_id, child_pos),
  CONSTRAINT entity_group_pc_mk
             FOREIGN KEY (model_id, model_entity_id, group_id) REFERENCES entity_group_lst (model_id, model_entity_id, group_id)
);

-- next db schema version
UPDATE id_lst SET id_value = 105 WHERE id_key = 'openmpp';

COMMIT;
