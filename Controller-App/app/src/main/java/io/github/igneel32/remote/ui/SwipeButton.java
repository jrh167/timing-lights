package io.github.igneel32.remote.ui;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.animation.ValueAnimator;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.core.content.ContextCompat;

import io.github.igneel32.remote.MainActivity;
import io.github.igneel32.remote.R;
import io.github.igneel32.remote.serial.SerialState;


public class SwipeButton extends RelativeLayout {
    private ImageView slidingButton;
    private float initialX;
    private boolean active;
    private int initialButtonWidth;
    private TextView centerText;
    private boolean shouldSlide = false;

    private Drawable warningDrawable;

    public SwipeButton(Context context) {
        super(context);

        init(context, null, -1, -1);
    }

    public SwipeButton(Context context, AttributeSet attrs) {
        super(context, attrs);

        init(context, attrs, -1, -1);
    }

    public SwipeButton(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);

        init(context, attrs, defStyleAttr, -1);
    }

    @TargetApi(21)
    public SwipeButton(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init(context, attrs, defStyleAttr, defStyleRes);
    }

    private void init(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        RelativeLayout background = new RelativeLayout(context);

        LayoutParams layoutParamsView = new LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);

        layoutParamsView.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);

        background.setBackground(ContextCompat.getDrawable(context, R.drawable.shape_rounded));

        addView(background, layoutParamsView);
        final TextView centerText = new TextView(context);
        this.centerText = centerText;

        LayoutParams layoutParams = new LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);

        layoutParams.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);
        centerText.setText(getResources().getString(R.string.emergency_stop));
        centerText.setTextColor(Color.WHITE);
        centerText.setPadding(25, 25, 25, 25);
        background.addView(centerText, layoutParams);

        final ImageView swipeButton = new ImageView(context);
        this.slidingButton = swipeButton;

        warningDrawable = ContextCompat.getDrawable(getContext(), R.drawable.ic_baseline_warning_24);

        slidingButton.setImageDrawable(warningDrawable);
        slidingButton.setPadding(30, 30, 30, 30);

        LayoutParams layoutParamsButton = new LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);

        layoutParamsButton.addRule(RelativeLayout.ALIGN_PARENT_LEFT, RelativeLayout.TRUE);
        layoutParamsButton.addRule(RelativeLayout.CENTER_VERTICAL, RelativeLayout.TRUE);
        swipeButton.setBackground(ContextCompat.getDrawable(context, R.drawable.shape_button));
        swipeButton.setImageDrawable(warningDrawable);
        addView(swipeButton, layoutParamsButton);

        setOnTouchListener(getButtonTouchListener());
    }

    @SuppressLint("ClickableViewAccessibility")
    private OnTouchListener getButtonTouchListener() {
        return (v, event) -> {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    shouldSlide = event.getX() <= slidingButton.getWidth() &&
                            event.getX() > slidingButton.getX();
                    return true;
                case MotionEvent.ACTION_MOVE:
                    if (!shouldSlide) return true;

                    if (initialX == 0) {
                        initialX = slidingButton.getX();
                    }
                    if (event.getX() > initialX + slidingButton.getWidth() / 2.0 &&
                            event.getX() + slidingButton.getWidth() / 2.0 < getWidth()) {
                        slidingButton.setX((float) (event.getX() - slidingButton.getWidth() / 2.0));
                        centerText.setAlpha(1 - 1.3f * (slidingButton.getX() + slidingButton.getWidth()) / getWidth());
                    }

                    if (event.getX() + slidingButton.getWidth() / 2.0 > getWidth() &&
                            slidingButton.getX() + slidingButton.getWidth() / 2.0 < getWidth()) {
                        slidingButton.setX(getWidth() - slidingButton.getWidth());
                    }

                    if (event.getX() < slidingButton.getWidth() / 2.0 &&
                            slidingButton.getX() > 0) {
                        slidingButton.setX(0);
                    }
                    return true;
                case MotionEvent.ACTION_UP:
                    if (active) {
                        collapseButton();
                    } else {
                        initialButtonWidth = slidingButton.getWidth();

                        if (slidingButton.getX() + slidingButton.getWidth() > getWidth() * 0.85) {
                            runEmergencyStop();
                        }
                        moveButtonBack();
                    }

                    return true;
            }

            return false;
        };
    }

    private void collapseButton() {
        final ValueAnimator widthAnimator = ValueAnimator.ofInt(
                slidingButton.getWidth(),
                initialButtonWidth);

        widthAnimator.addUpdateListener(animation -> {
            ViewGroup.LayoutParams params =  slidingButton.getLayoutParams();
            params.width = (Integer) widthAnimator.getAnimatedValue();
            slidingButton.setLayoutParams(params);
        });

        widthAnimator.addListener(new AnimatorListenerAdapter() {
            @Override
            public void onAnimationEnd(Animator animation) {
                super.onAnimationEnd(animation);
                active = false;
                slidingButton.setImageDrawable(warningDrawable);
            }
        });

        ObjectAnimator objectAnimator = ObjectAnimator.ofFloat(
                centerText, "alpha", 1);

        AnimatorSet animatorSet = new AnimatorSet();

        animatorSet.playTogether(objectAnimator, widthAnimator);
        animatorSet.start();
    }

    private void moveButtonBack() {
        final ValueAnimator positionAnimator =
                ValueAnimator.ofFloat(slidingButton.getX(), 0);
        positionAnimator.setInterpolator(new AccelerateDecelerateInterpolator());
        positionAnimator.addUpdateListener(animation -> {
            float x = (Float) positionAnimator.getAnimatedValue();
            slidingButton.setX(x);
        });

        ObjectAnimator objectAnimator = ObjectAnimator.ofFloat(
                centerText, "alpha", 1);

        positionAnimator.setDuration(200);

        AnimatorSet animatorSet = new AnimatorSet();
        animatorSet.playTogether(objectAnimator, positionAnimator);
        animatorSet.start();
    }

    private Activity getActivity() {
        Context context = getContext();
        while (context instanceof ContextWrapper) {
            if (context instanceof Activity) {
                return (Activity)context;
            }
            context = ((ContextWrapper)context).getBaseContext();
        }
        return null;
    }

    private void runEmergencyStop() {
        MainActivity mainActivity = (MainActivity) getActivity();
        if (mainActivity == null) return;  // This case should never occur
        SerialState state = mainActivity.getSerialState();
        state.setEmergencyStop(true);
        state.setCountdown(false);
        state.setTimeEnabled(false);
        int current = mainActivity.navController.getCurrentDestination().getId();
        if (current == R.id.navigation_target) {
            state.setTime(mainActivity.getStorage().getValue().getTargetMaxTime());
        } else if (current == R.id.navigation_matchplay) {
            state.setTime(mainActivity.getStorage().getValue().getMatchplayMaxTime());
        }
        state.setColour(SerialState.Colour.RED);
        mainActivity.getSerialComms().sendState();
        mainActivity.setCountDownNumber(-1);
        Toast.makeText(mainActivity, "Triggered emergency stop!", Toast.LENGTH_LONG).show();
    }
}