Library Management System (LMS)
A clean, beginner‑friendly LMS that lets users search books, check real‑time availability (stock and next‑available date/ETA) and price, and perform actions: Issue, Return, Renew, Place Hold, and optional Buy. This repo starts with a C (C11) CLI prototype and is structured to grow into a web app later. No deploys yet—local setup instructions below.

Status

Active development (MVP in progress)
No live deployment yet (Netlify/GitHub Pages planned)
Features (MVP scope)
Search by book title (author/ISBN soon)
Show stock, price, rental fee/day, and earliest next‑available date
Circulation: Issue/Return/Renew with due dates and basic fines
Staff view (later): current borrowers and loan windows
Holds/Reservations (later): FIFO queue with expiry
Optional Buy flow (later)
Audit logging and RBAC/auth (later)
Tech Stack
Prototype: C (C11) CLI
Planned web stack: React (Vite) + Netlify Functions (Node 18) + Postgres (Supabase/Neon)
Repository Structure
cli/lms.c — C CLI prototype (self‑contained with seed data)
web/ — (planned) React app
netlify/functions/ — (planned) serverless API
db/migrations/ — (planned) SQL migrations and seeds
tests/ — (planned) unit/integration/e2e
If a folder doesn’t exist yet, it’s scheduled for a later milestone.

Getting Started (CLI)
Requirements
GCC/Clang (C11), Make (optional), a terminal
Build and run
      git clone https://github.com/sumitsaxena2716-sys/library-management-system
      cd library-management-system
      
      # Compile
      gcc -std=c11 -O2 cli/lms.c -o lms
      
      # Run
      ./lms
What you can try

Search book → enter a title fragment
Select a book → if in stock, choose Issue or Buy
Return a book → use an active Loan ID (seeded IDs: 1, 2, 3)
Notes

Seed data is embedded in cli/lms.c (books, members, loans, sales).
Loan IDs auto‑increment as you issue new loans.
Roadmap
Short term: availability engine polish, transaction‑safe Issue/Return/Renew, Holds FIFO, Auth/RBAC
Mid term: REST API (Netlify Functions), React UI, reports/CSV export
Long term: search FTS, barcode/QR, monitoring, backups, and deployment to Netlify + GitHub Pages (UI mirror)
Contributing
Branch naming: feature/<name>, fix/<name>, docs/<name>
Open an issue for bugs/enhancements; PRs welcome once CI is added
License
MIT (recommended). Add LICENSE before first release.
Team
Lead: Aisha Fatima
Members: Priyanshu Singh Fartiyal, Sumit Saxena

GCC/Clang (C11), Make (optional), a terminal
Build and run
