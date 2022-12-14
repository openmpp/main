BEGIN EXCLUSIVE TRANSACTION;

--
-- Entities
--
CREATE TABLE IF NOT EXISTS entity_dic
(
  entity_hid    INT          NOT NULL, -- unique entity id
  entity_name   VARCHAR(255) NOT NULL, -- entity name
  entity_digest VARCHAR(32)  NOT NULL, -- entity digest
  PRIMARY KEY (entity_hid),
  CONSTRAINT entity_dic_un UNIQUE (entity_digest)
);

--
-- Model entities
--
CREATE TABLE IF NOT EXISTS model_entity_dic
(
  model_id        INT      NOT NULL, -- master key
  model_entity_id INT      NOT NULL, -- model entity id
  entity_hid      INT      NOT NULL, -- master key
  PRIMARY KEY (model_id, model_entity_id),
  CONSTRAINT model_entity_dic_un UNIQUE (model_id, entity_hid),
  CONSTRAINT model_entity_dic_mk
             FOREIGN KEY (entity_hid) REFERENCES entity_dic (entity_hid),
  CONSTRAINT model_entity_dic_fk
             FOREIGN KEY (model_id) REFERENCES model_dic (model_id)
);

--
-- Entity text info: description and notes
--
CREATE TABLE IF NOT EXISTS entity_dic_txt
(
  entity_hid INT            NOT NULL, -- master key
  lang_id    INT            NOT NULL, -- language id
  descr      VARCHAR(255)   NOT NULL, -- entity description
  note       VARCHAR(32000),          -- entity notes
  PRIMARY KEY (entity_hid, lang_id),
  CONSTRAINT entity_dic_txt_mk
             FOREIGN KEY (entity_hid) REFERENCES entity_dic (entity_hid),
  CONSTRAINT entity_dic_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Entity attributes
--
CREATE TABLE IF NOT EXISTS entity_attr
(
  entity_hid  INT          NOT NULL, -- master key
  attr_id     INT          NOT NULL, -- attribute index
  attr_name   VARCHAR(255) NOT NULL, -- attribute name: Year
  type_hid    INT          NOT NULL, -- attribute type
  is_internal SMALLINT     NOT NULL, -- if non-zero then attribute is internal, e.g.: om_attr_year
  PRIMARY KEY (entity_hid, attr_id),
  CONSTRAINT entity_attr_mk
             FOREIGN KEY (entity_hid) REFERENCES entity_dic (entity_hid),
  CONSTRAINT entity_attr_type_fk
             FOREIGN KEY (type_hid) REFERENCES type_dic (type_hid)
);

--
-- Entity attributes text info: description and notes
--
CREATE TABLE IF NOT EXISTS entity_attr_txt
(
  entity_hid INT            NOT NULL, -- master key
  attr_id    INT            NOT NULL, -- master key
  lang_id    INT            NOT NULL, -- language id
  descr      VARCHAR(255)   NOT NULL, -- entity attribute description
  note       VARCHAR(32000),          -- entity attribute notes
  PRIMARY KEY (entity_hid, attr_id, lang_id),
  CONSTRAINT entity_attr_txt_mk
             FOREIGN KEY (entity_hid, attr_id) REFERENCES entity_attr (entity_hid, attr_id),
  CONSTRAINT entity_attr_txt_lang_fk
             FOREIGN KEY (lang_id) REFERENCES lang_lst (lang_id)
);

--
-- Entity generation:
-- partial instance of entity, may include only some of entity attributes
--
CREATE TABLE IF NOT EXISTS entity_gen
(
  entity_gen_hid  INT         NOT NULL, -- entity generation unique id
  entity_hid      INT         NOT NULL, -- master key
  db_entity_table VARCHAR(64) NOT NULL, -- db table name: Person_g87abcdef or Person_e87abcdef
  gen_digest      VARCHAR(32) NOT NULL, -- digest of entity generation
  PRIMARY KEY (entity_gen_hid),
  CONSTRAINT entity_gen_un UNIQUE (db_entity_table),
  CONSTRAINT entity_gen_fk
             FOREIGN KEY (entity_hid) REFERENCES entity_dic (entity_hid)
);

--
-- Entity generation attributes
--
CREATE TABLE IF NOT EXISTS entity_gen_attr
(
  entity_gen_hid INT NOT NULL, -- master key
  attr_id        INT NOT NULL, -- attribute index
  entity_hid     INT NOT NULL, -- entity unique id
  PRIMARY KEY (entity_gen_hid, attr_id),
  CONSTRAINT entity_gen_attr_mk
             FOREIGN KEY (entity_gen_hid) REFERENCES entity_gen (entity_gen_hid),
  CONSTRAINT entity_gen_attr_fk
             FOREIGN KEY (entity_hid, attr_id) REFERENCES entity_attr (entity_hid, attr_id)
);

--
-- Run entities list
-- "base run id" is run id where values of entity attribute values must be selected from
--
CREATE TABLE IF NOT EXISTS run_entity
(
  run_id         INT         NOT NULL, -- master key
  entity_gen_hid INT         NOT NULL, -- entity generation unique id
  base_run_id    INT         NOT NULL, -- source run id to select entity attribute values
  value_digest   VARCHAR(32),          -- if not NULL then digest of attribute values for the run
  PRIMARY KEY (run_id, entity_gen_hid),
  CONSTRAINT run_entity_mk
             FOREIGN KEY (run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_entity_base_fk
             FOREIGN KEY (base_run_id) REFERENCES run_lst (run_id),
  CONSTRAINT run_entity_gen_fk
             FOREIGN KEY (entity_gen_hid) REFERENCES entity_gen (entity_gen_hid)
);

INSERT INTO id_lst 
  (id_key, id_value)
SELECT 
  'entity_hid', 100
FROM id_lst IL
WHERE NOT EXISTS
(
  SELECT * FROM id_lst E WHERE E.id_key = 'entity_hid'
)
AND IL.id_key =
(
  SELECT MIN(M.id_key) from id_lst M
);

UPDATE id_lst SET id_value = 103 WHERE id_key = 'openmpp';

COMMIT;
