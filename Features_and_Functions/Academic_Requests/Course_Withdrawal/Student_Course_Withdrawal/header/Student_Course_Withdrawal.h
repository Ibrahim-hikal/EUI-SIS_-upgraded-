/*
* File: Student_Course_Withdrawal.h
 * Description: Sets up the tool that allows students to ask permission to drop
 * a class they are currently taking.
 */

#ifndef STUDENT_COURSE_WITHDRAWAL_H
#define STUDENT_COURSE_WITHDRAWAL_H

#include <string>

class StudentWithdrawalManager {
public:
    /*
     * Takes the student's ID, the class they want to drop, and their reason,
     * and saves it so the Admin can review it later.
     */
    static void submit(const std::string& student_id, const std::string& course_code, const std::string& reason);
};

#endif