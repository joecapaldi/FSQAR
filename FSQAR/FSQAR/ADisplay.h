
#ifndef __ADISPLAY_H__
#define __ADISPLAY_H__

class ADisplay : public Window
{
public:
	ADisplay();
	virtual ~ADisplay();

	BOOL Create(const Window* pFrameWnd, LPCRECT lpRect);

	void OnCreate();
	void OnPaint(HDC hDC);
protected:
private:
};

#endif // __ADISPLAY_H__