source("../../build/src/test/loadtestenv.m");
system("../../build/src/mvp/Frontend/mvpdumpjob ../real/newconf.mvp 2682 1937 12");

job = loadjobfile("2682_1937_12.job");
