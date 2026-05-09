/*
* File: Admin_Course_Withdrawal.h
 * Description: Sets up the tools for the administrator to review student withdrawal
 * requests. It defines how a request looks and creates the functions to process it.
 */

#ifndef ADMIN_COURSE_WITHDRAWAL_H
#define ADMIN_COURSE_WITHDRAWAL_H

#include <string>
#include <vector>

// Renamed from WithdrawalRequest to WithdrawalData to avoid clashing with the UI
struct WithdrawalData {
    std::string student_id;
    std::string course_code;
    std::string reason;
};

class AdminWithdrawalManager {
public:
    // Reads all the pending requests from the database so the Admin can see them
    static std::vector<WithdrawalData> fetch();

    /*
     * Handles the Admin's decision.
     * If approved is true, it removes the class from the student's record.
     * Either way, it deletes the request from the pending list.
     */
    static void process(const std::string& student_id, const std::string& course_code, bool approved);
};

#endif