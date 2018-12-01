create table "creditTime" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "creditTimes_id" bigint,
  "start" text,
  "stop" text,
  "hasClockedOut" boolean not null,
  constraint "fk_creditTime_creditTimes" foreign key ("creditTimes_id") references "user" ("id") deferrable initially deferred
);
create table "user" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "role" integer not null,
  "name" text not null
);
create table "absence" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "absences_id" bigint,
  "first" text,
  "last" text,
  "reason" integer not null,
  constraint "fk_absence_absences" foreign key ("absences_id") references "user" ("id") deferrable initially deferred
);
create table "holiday" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "first" text,
  "last" text
);
create table "debitTime" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "debitTimes_id" bigint,
  "validFrom" text,
  "workHoursPerWeekday_0" double precision not null,
  "workHoursPerWeekday_1" double precision not null,
  "workHoursPerWeekday_2" double precision not null,
  "workHoursPerWeekday_3" double precision not null,
  "workHoursPerWeekday_4" double precision not null,
  "workHoursPerWeekday_5" double precision not null,
  "workHoursPerWeekday_6" double precision not null,
  constraint "fk_debitTime_debitTimes" foreign key ("debitTimes_id") references "user" ("id") deferrable initially deferred
);
create table "auth_identity" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "auth_info_id" bigint,
  "provider" varchar(64) not null,
  "identity" varchar(512) not null,
  constraint "fk_auth_identity_auth_info" foreign key ("auth_info_id") references "auth_info" ("id") on delete cascade deferrable initially deferred
);
create table "auth_info" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "user_id" bigint,
  "password_hash" varchar(100) not null,
  "password_method" varchar(20) not null,
  "password_salt" varchar(20) not null,
  "status" integer not null,
  "failed_login_attempts" integer not null,
  "last_login_attempt" text,
  "email" varchar(256) not null,
  "unverified_email" varchar(256) not null,
  "email_token" varchar(64) not null,
  "email_token_expires" text,
  "email_token_role" integer not null,
  constraint "fk_auth_info_user" foreign key ("user_id") references "user" ("id") on delete cascade deferrable initially deferred
);
create table "auth_token" (
  "id" integer primary key autoincrement,
  "version" integer not null,
  "auth_info_id" bigint,
  "value" varchar(64) not null,
  "expires" text,
  constraint "fk_auth_token_auth_info" foreign key ("auth_info_id") references "auth_info" ("id") on delete cascade deferrable initially deferred
);
