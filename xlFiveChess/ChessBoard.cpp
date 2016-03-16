//------------------------------------------------------------------------------
//
//    Copyright (C) Streamlet. All rights reserved.
//
//    File Name:   ChessBoard.cpp
//    Author:      Streamlet
//    Create Time: 2016-03-16
//    Description: 
//
//------------------------------------------------------------------------------


#include "ChessBoard.h"
#include <xl/Windows/GUI/xlDPI.h>
#include <windowsx.h>

ChessBoard::ChessBoard() : m_nBlockSize(0), m_nLeftBlank(0), m_nTopBlank(0)
{
    AppendMsgHandler(WM_CREATE, MsgHandler(this, &ChessBoard::OnCreate));
    AppendMsgHandler(WM_ERASEBKGND, MsgHandler(this, &ChessBoard::OnEraseBkgnd));
    AppendMsgHandler(WM_PAINT, MsgHandler(this, &ChessBoard::OnPaint));
    //AppendMsgHandler(WM_WINDOWPOSCHANGING, MsgHandler(this, &ChessBoard::OnWindowPosChanging));
    AppendMsgHandler(WM_SIZE, MsgHandler(this, &ChessBoard::OnSize));

    AppendMsgHandler(WM_LBUTTONDOWN, MsgHandler(this, &ChessBoard::OnLButtonDown));
    AppendMsgHandler(WM_LBUTTONUP, MsgHandler(this, &ChessBoard::OnLButtonUp));
    AppendMsgHandler(WM_MOUSEMOVE, MsgHandler(this, &ChessBoard::OnMouseMove));

    ZeroMemory(&m_ptMouseDown, sizeof(m_ptMouseDown));

    m_ptPreMove.x = -1;
    m_ptPreMove.y = -1;
}

ChessBoard::~ChessBoard()
{

}

void ChessBoard::InitializeCoord()
{
    RECT rc = {};
    GetClientRect(&rc);

    m_nBlockSize = min(rc.right - rc.left, rc.bottom - rc.top) / m_nChessBoardSize;
    int nTotalSize = m_nBlockSize * m_nChessBoardSize;
    m_nLeftBlank = ((rc.right - rc.left) - nTotalSize) / 2;
    m_nTopBlank = ((rc.bottom - rc.top) - nTotalSize) / 2;
}

bool ChessBoard::LogicalToPhysical(POINT &pt)
{
    if (pt.x < 0 || pt.x >= m_nChessBoardSize || pt.y < 0 || pt.y >= m_nChessBoardSize)
    {
        return false;
    }

    pt.x = m_nLeftBlank + pt.x * m_nBlockSize + m_nBlockSize / 2;
    pt.y = m_nTopBlank + pt.y * m_nBlockSize + m_nBlockSize / 2;

    return true;
}

bool ChessBoard::PhysicalToLogical(POINT &pt)
{
    int nTotalSize = m_nBlockSize * m_nChessBoardSize;

    if (pt.x < m_nLeftBlank || pt.x >= m_nLeftBlank + nTotalSize || pt.y < m_nTopBlank || pt.y >= m_nTopBlank + nTotalSize)
    {
        return false;
    }

    pt.x = (pt.x - m_nLeftBlank) / m_nBlockSize;
    pt.y = (pt.y - m_nTopBlank) / m_nBlockSize;

    return true;
}

void ChessBoard::DrawChessBoard(HDC hDC)
{
    HPEN hPen = (HPEN)GetStockObject(BLACK_PEN);
    SelectPen(hDC, hPen);

    for (int i = 0; i < m_nChessBoardSize; ++i)
    {
        POINT pt1 = { i, 0 }, pt2 = { i, m_nChessBoardSize - 1 };
        LogicalToPhysical(pt1);
        LogicalToPhysical(pt2);
        MoveToEx(hDC, (pt1.x), (pt1.y), NULL);
        LineTo(hDC, (pt2.x), (pt2.y));

        POINT pt3 = { 0, i }, pt4 = { m_nChessBoardSize - 1, i };
        LogicalToPhysical(pt3);
        LogicalToPhysical(pt4);
        MoveToEx(hDC, (pt3.x), (pt3.y), NULL);
        LineTo(hDC, (pt4.x), (pt4.y));
    }
}

void ChessBoard::DrawChessMan(HDC hDC, POINT pt, bool bBlack)
{
    LogicalToPhysical(pt);
    int nDelta = m_nBlockSize / 3;

    HBRUSH hBrush = (HBRUSH)GetStockObject(bBlack ? BLACK_BRUSH : WHITE_BRUSH);
    SelectBrush(hDC, hBrush);
    Ellipse(hDC, pt.x - nDelta, pt.y - nDelta, pt.x + nDelta, pt.y + nDelta);
}


LRESULT ChessBoard::OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    InitializeCoord();
    m_FiveChess.NewGame();
    return 0;
}

LRESULT ChessBoard::OnEraseBkgnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    return TRUE;
}

LRESULT ChessBoard::OnPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    RECT rc = {};
    GetClientRect(&rc);

    PAINTSTRUCT ps = {};
    BeginPaint(&ps);

    HDC hDC = CreateCompatibleDC(ps.hdc);
    HBITMAP hBmp = CreateCompatibleBitmap(hDC, rc.right - rc.left, rc.bottom - rc.top);
    SelectBitmap(hDC, hBmp);

    SetBkColor(hDC, RGB(0xff, 0xff, 0xff));
    ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

    DrawChessBoard(hDC);

    const FiveChess::ChessData &data = m_FiveChess.GetChessData();

    for (int i = 0; i < m_nChessBoardSize; ++i)
    {
        for (int j = 0; j < m_nChessBoardSize; ++j)
        {
            if (data[i][j] != FiveChess::None)
            {
                POINT pt = { i, j };
                DrawChessMan(hDC, pt, data[i][j] == FiveChess::Black);
            }
        }
    }

    if (m_FiveChess.WhoseTurn() == FiveChess::Black && m_ptPreMove.x != -1 && m_ptPreMove.y != -1)
    {
        DrawChessMan(hDC, m_ptPreMove, true);
    }

    BitBlt(ps.hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hDC, 0, 0, SRCCOPY);

    DeleteObject(hBmp);
    DeleteObject(hDC);

    EndPaint(&ps);

    return 0;
}

LRESULT ChessBoard::OnWindowPosChanging(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    return 0;
}

LRESULT ChessBoard::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    InitializeCoord();
    return 0;
}

LRESULT ChessBoard::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    SetCapture();

    m_ptMouseDown.x = GET_X_LPARAM(lParam);
    m_ptMouseDown.y = GET_Y_LPARAM(lParam);

    return 0;
}

LRESULT ChessBoard::OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    ReleaseCapture();

    if (m_FiveChess.WhoseTurn() != FiveChess::Black)
    {
        return 0;
    }

    if (abs(GET_X_LPARAM(lParam) - m_ptMouseDown.x) > GetSystemMetrics(SM_CXDRAG) ||
        abs(GET_Y_LPARAM(lParam) - m_ptMouseDown.y) > GetSystemMetrics(SM_CYDRAG))
    {
        return 0;
    }

    POINT pt = m_ptMouseDown;
    PhysicalToLogical(pt);

    m_FiveChess.Move(pt.x, pt.y, FiveChess::Black);
    m_FiveChess.AutoMove(FiveChess::White);

    Invalidate();

    return 0;
}

LRESULT ChessBoard::OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    if (PhysicalToLogical(pt))
    {
        m_ptPreMove = pt;
    }
    else
    {
        m_ptPreMove.x = -1;
        m_ptPreMove.y = -1;
    }

    Invalidate();

    return 0;
}