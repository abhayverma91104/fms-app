# fms-app
A terminal based Flight Management System built in C++ with user/admin login, flight management, seat booking, cancellations, waitlists, loyalty points, and persistent storage using BSTs and STL structures. Designed for reliability, scalability, and efficient data handling.
# 🚀 Features
# User Panel
1. Register and log in.
2. Search available flights.
3. Book seats with optional seat preference.
4. Automatic seat assignment.
5. Cancel bookings anytime.
6. Waitlist support with automatic allocation.
7. View booking history.
8. Earn and track loyalty points.
   
# Admin Panel
1. Add, edit, and remove flights.
2. View all flights and seat availability.
3. Inspect waitlists and seat maps.
4. View all users and their activity.
5. Force-cancel bookings.
6. Export detailed flight reports.
   
# 🧠 Technical Details
1. Binary Search Tree (BST) for flight indexing.
2. Linked List–based waitlist system.
3. Extensive use of Vectors, Maps, and other STL structures.
4. Persistent storage using .dat files.
5. Input-safe custom wrappers for clean user interaction.
6. Fully contained in a single .cpp file for portability.

# 🔧 Compile and Run (macOS / Linux)
g++ -std=c++17 Flight_management_system.cpp -o FlightApp
./FlightApp

# ▶️ Compile and Run (Windows)
g++ -std=c++17 Flight_management_system.cpp -o FlightApp.exe
FlightApp.exe

# 📌 Requirements
C++17-compatible compiler (g++, clang++, or MSVC).
Terminal or command prompt environment.

# 📝 Future Improvements
1. Enhanced admin analytics.
2. Role-based controls for airlines.
3. API compatibility layer.
4. GUI version with Qt or ImGui.
5. Secure password hashing.
