create table "creditTime" (
  "id" BIGINT primary key AUTO_INCREMENT,
  "version" integer not null,
  "creditTimes_id" bigint,
  "start" datetime,
  "stop" datetime,
  "hasClockedOut" boolean not null
);
create table "annualStatement" (
  "id" BIGINT primary key AUTO_INCREMENT,
  "version" integer not null,
  "annualStatements_id" bigint,
  "referenceDate" date,
  "balance" integer not null
);
create table "user" (
  "id" BIGINT primary key AUTO_INCREMENT,
  "version" integer not null,
  "role" integer not null,
  "name" text not null
);
create table "absence" (
  "id" BIGINT primary key AUTO_INCREMENT,
  "version" integer not null,
  "absences_id" bigint,
  "first" date,
  "last" date,
  "reason" integer not null
);
create table "holiday" (
  "id" BIGINT primary key AUTO_INCREMENT,
  "version" integer not null,
  "first" date,
  "last" date
);
create table "debitTime" (
  "id" BIGINT primary key AUTO_INCREMENT,
  "version" integer not null,
  "debitTimes_id" bigint,
  "validFrom" date,
  "workHoursPerWeekday_0" double precision not null,
  "workHoursPerWeekday_1" double precision not null,
  "workHoursPerWeekday_2" double precision not null,
  "workHoursPerWeekday_3" double precision not null,
  "workHoursPerWeekday_4" double precision not null,
  "workHoursPerWeekday_5" double precision not null,
  "workHoursPerWeekday_6" double precision not null
);
create table "auth_identity" (
  "id" BIGINT primary key AUTO_INCREMENT,
  "version" integer not null,
  "auth_info_id" bigint,
  "provider" varchar(64) not null,
  "identity" varchar(512) not null
);
create table "auth_info" (
  "id" BIGINT primary key AUTO_INCREMENT,
  "version" integer not null,
  "user_id" bigint,
  "password_hash" varchar(100) not null,
  "password_method" varchar(20) not null,
  "password_salt" varchar(20) not null,
  "status" integer not null,
  "failed_login_attempts" integer not null,
  "last_login_attempt" datetime,
  "email" varchar(256) not null,
  "unverified_email" varchar(256) not null,
  "email_token" varchar(64) not null,
  "email_token_expires" datetime,
  "email_token_role" integer not null
);
create table "auth_token" (
  "id" BIGINT primary key AUTO_INCREMENT,
  "version" integer not null,
  "auth_info_id" bigint,
  "value" varchar(64) not null,
  "expires" datetime
);
alter table "creditTime" add constraint "fk_creditTime_creditTimes" foreign key ("creditTimes_id") references "user" ("id");
alter table "annualStatement" add constraint "fk_annualStatement_annualStatements" foreign key ("annualStatements_id") references "user" ("id");
alter table "absence" add constraint "fk_absence_absences" foreign key ("absences_id") references "user" ("id");
alter table "debitTime" add constraint "fk_debitTime_debitTimes" foreign key ("debitTimes_id") references "user" ("id");
alter table "auth_identity" add constraint "fk_auth_identity_auth_info" foreign key ("auth_info_id") references "auth_info" ("id") on delete cascade;
alter table "auth_info" add constraint "fk_auth_info_user" foreign key ("user_id") references "user" ("id") on delete cascade;
alter table "auth_token" add constraint "fk_auth_token_auth_info" foreign key ("auth_info_id") references "auth_info" ("id") on delete cascade;
