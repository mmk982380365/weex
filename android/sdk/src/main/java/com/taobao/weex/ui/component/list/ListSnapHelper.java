/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.taobao.weex.ui.component.list;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.widget.OrientationHelper;
import android.support.v7.widget.PagerSnapHelper;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;

import com.taobao.weex.common.Constants;

public class ListSnapHelper extends PagerSnapHelper {
    OrientationHelper mVerticalHelper;
    int mPaddingTop = 0;
    int mPaddingBottom = 0;
    public final static int SNAP_ALIGN_START = 0;
    public final static int SNAP_ALIGN_CENTER = 1;
    public final static int SNAP_ALIGN_BOTTOM = 2;
    private int mAlign = 0;
    private int mSnapPositon = 0;
    private int mSnapTrigger = 50;
    private int mSnapTriggerReverse = 50;


    public void setPaddingTop(int paddingTop) {
        this.mPaddingTop = paddingTop;
    }

    public void setPaddingBottom(int paddingBottom) {
        this.mPaddingBottom = paddingBottom;
    }

    public void setTrigger(int trigger) {
        this.mSnapTrigger = trigger;
    }

    public void setTriggerReverse(int trigger) {
        this.mSnapTriggerReverse = trigger;
    }

    public void setSnapAlign(int align) {
        this.mAlign = align;
    }

    public int getSnapPosition(){
        return mSnapPositon;
    }

    private int getContainerBaseLine(RecyclerView.LayoutManager layoutManager, int align) {
        OrientationHelper helper = getVerticalHelper(layoutManager);
        switch (align) {
            case SNAP_ALIGN_START:
                if (layoutManager.getClipToPadding()) {
                    return helper.getStartAfterPadding() + mPaddingTop;
                } else {
                    return mPaddingTop;
                }
            case SNAP_ALIGN_CENTER:
                if (layoutManager.getClipToPadding()) {
                    return helper.getStartAfterPadding() + mPaddingTop + (helper.getTotalSpace() - mPaddingTop - mPaddingBottom) / 2;
                } else {
                    return (helper.getEnd() - mPaddingTop - mPaddingBottom) / 2 + mPaddingTop;
                }
            case SNAP_ALIGN_BOTTOM:
                if (layoutManager.getClipToPadding()) {
                    return helper.getEndAfterPadding() - mPaddingBottom;
                } else {
                    return helper.getEnd() - mPaddingBottom;
                }
            default:
                return 0;
        }
    }

    private int getChildBaseLine(RecyclerView.LayoutManager layoutManager, View child, int align) {
        OrientationHelper helper = getVerticalHelper(layoutManager);
        switch (align) {
            case SNAP_ALIGN_START:
                return helper.getDecoratedStart(child);
            case SNAP_ALIGN_CENTER:
                return helper.getDecoratedStart(child)
                        + helper.getDecoratedMeasurement(child) / 2;
            case SNAP_ALIGN_BOTTOM:
                return helper.getDecoratedEnd(child);
            default:
                return 0;
        }
    }


    @Nullable
    @Override
    public View findSnapView(RecyclerView.LayoutManager layoutManager) {
        int childCount = layoutManager.getChildCount();
        if (childCount == 0) {
            return null;
        }

        View closestChild = null;
        final int containerBaseLine = getContainerBaseLine(layoutManager, mAlign);

        int absClosest = Integer.MAX_VALUE;
        int disClosest = Integer.MAX_VALUE;
        int closetIndex = 0;
        for (int i = 0; i < childCount; i++) {
            final View child = layoutManager.getChildAt(i);
            int childBaseLine = getChildBaseLine(layoutManager, child, mAlign);
            int distance = childBaseLine - containerBaseLine;
            int absDistance = Math.abs(distance);

            if (absDistance < absClosest) {
                absClosest = absDistance;
                closestChild = child;
                disClosest = distance;
                closetIndex = i;
            }
        }
        if(TextUtils.equals(Constants.Name.SCROLL_SNAP_IGNORE, String.valueOf(closestChild.getTag()))){
            return null;
        }
        if(closestChild != null){
            int scrollOffset = disClosest < 0 ? absClosest - mSnapTrigger : absClosest - mSnapTriggerReverse;
            if(scrollOffset > 0 && layoutManager.getPosition(layoutManager.getChildAt(0)) > 0 && layoutManager.getPosition(layoutManager.getChildAt(childCount - 1)) < layoutManager.getItemCount() - 1){
                View tempCild;
                if(disClosest < 0){
                    tempCild = layoutManager.getChildAt(closetIndex + 1);
                }
                else{
                    tempCild = layoutManager.getChildAt(closetIndex - 1);
                }
                if(tempCild != null) {
                    closestChild = tempCild;
                }
            }
            mSnapPositon = layoutManager.getPosition(closestChild);
        }
        return closestChild;
    }

    private OrientationHelper getVerticalHelper(RecyclerView.LayoutManager layoutManager) {
        if (mVerticalHelper == null) {
            mVerticalHelper = OrientationHelper.createVerticalHelper(layoutManager);
        }
        return mVerticalHelper;
    }

    @Nullable
    @Override
    public int[] calculateDistanceToFinalSnap(@NonNull RecyclerView.LayoutManager layoutManager, @NonNull View targetView) {
        int[] out = new int[2];
        out[0] = 0;
        out[1] = getChildBaseLine(layoutManager, targetView, mAlign) - getContainerBaseLine(layoutManager, mAlign);
        return out;
    }
}
