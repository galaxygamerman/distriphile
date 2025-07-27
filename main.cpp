#include <iostream>
#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

const std::string CONNECTION_STRING =
"host=asia-south1.509ecc4c-201c-4248-a364-c059af51f5c4.gcp.yugabyte.cloud port=5433 user=admin password=gcW10R2_HiftI07pc-D0TzghDVs1mp dbname=yugabyte sslmode=verify-full sslrootcert=root.crt";

void createDatabase(Session& session);
void selectAccounts(Session& session);
void transferMoneyBetweenAccounts(Session& session, int amount);

int main(int argc, char* argv[]) {
    std::cout << "Welcome!" << std::endl;

    // Register the connector once at the start
    Poco::Data::PostgreSQL::Connector::registerConnector();

    try {
        std::cout << "Using connection string: " << CONNECTION_STRING << std::endl;
        std::cout << ">>>> Connecting to YugabyteDB!" << std::endl;

        // 1. Create the session directly on the stack. No 'new' needed.
        Session session("PostgreSQL", CONNECTION_STRING);

        std::cout << ">>>> Successfully connected to YugabyteDB!" << std::endl;

        // 2. Pass the session by reference (&) to your functions
        createDatabase(session);
        selectAccounts(session);
        transferMoneyBetweenAccounts(session, 800);
        selectAccounts(session);

    }
    // 3. Catch the specific Poco exception to get better error details
    catch (const Poco::Exception& e) {
        std::cerr << "Poco Error: " << e.displayText() << std::endl;
        Poco::Data::PostgreSQL::Connector::unregisterConnector();
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard Error: " << e.what() << std::endl;
        Poco::Data::PostgreSQL::Connector::unregisterConnector();
        return 1;
    }

    // Unregister the connector before exiting
    Poco::Data::PostgreSQL::Connector::unregisterConnector();
    return 0;
}

void createDatabase(Session& session) {
    std::cout << ">>>> Creating tables..." << std::endl;
    session << "DROP TABLE IF EXISTS DemoAccount", now;
    session << "CREATE TABLE DemoAccount ( \
                id int PRIMARY KEY, \
                name varchar, \
                age int, \
                country varchar, \
                balance int)", now;
    session << "INSERT INTO DemoAccount VALUES (1, 'Jessica', 28, 'USA', 10000)", now;
    session << "INSERT INTO DemoAccount VALUES (2, 'John', 28, 'Canada', 9000)", now;

    std::cout << ">>>> Successfully created table DemoAccount." << std::endl;
}

void selectAccounts(Session& session) {
    std::cout << ">>>> Selecting accounts:" << std::endl;

    Statement select(session);
    select << "SELECT name, age, country, balance FROM DemoAccount";
    select.execute();

    Poco::Data::RecordSet rs(select);

    for (size_t i = 0; i < rs.rowCount(); ++i) {
        std::string name = rs["name"].convert<std::string>();
        int age = rs["age"].convert<int>();
        std::string country = rs["country"].convert<std::string>();
        int balance = rs["balance"].convert<int>();

        std::cout << "name=" << name << ", "
            << "age=" << age << ", "
            << "country=" << country << ", "
            << "balance=" << balance << std::endl;
        rs.moveNext();
    }
}

void transferMoneyBetweenAccounts(Session& session, int amount) {
    std::cout << ">>>> Transferring " << amount << " between accounts..." << std::endl;
    try {
        session << "UPDATE DemoAccount SET balance = balance -" + std::to_string(amount) +
            " WHERE name = 'Jessica'", now;
        session << "UPDATE DemoAccount SET balance = balance +" + std::to_string(amount) +
            " WHERE name = 'John'", now;

        std::cout << ">>>> Transferred " << amount << " between accounts." << std::endl;
    }
    catch (const Poco::Exception& e) {
        std::cerr << "Database error: " << e.displayText() << std::endl;
        throw;
    }
}