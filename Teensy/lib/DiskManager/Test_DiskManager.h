#include <DiskManager.h>
#include <unity.h>

void test_disk_creation() {
    DiskManager diskManager;

    diskManager.go_to_root();

    RunData run_data;
    run_data.datetime = now();

    TEST_ASSERT_TRUE(diskManager.create_run_folder("AlsoTesting"));
    diskManager.create_header_file(&run_data);
    
    diskManager.create_data_file();

    TEST_ASSERT_TRUE(true);

    int directory_num_init = diskManager.get_num_files();
    TEST_ASSERT_GREATER_OR_EQUAL(1, directory_num_init);

    String dir_name1 = diskManager.get_filename(0);
    Serial.println(dir_name1);

    String dir_name2 = diskManager.get_filename(1);
    Serial.println(dir_name2);

    String dir_name3 = diskManager.get_filename(2);
    Serial.println(dir_name3);

    diskManager.delete_run_folder("AlsoTesting");

    int directory_num_fin = diskManager.get_num_files();
    TEST_ASSERT_LESS_OR_EQUAL(directory_num_init, directory_num_fin);
}