/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.widget.quickactionsearchandbookmark;

import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.widget.RemoteViews;
import org.chromium.chrome.R;
import java.util.List;
import java.util.ArrayList;
import org.chromium.chrome.browser.profiles.Profile;
import android.content.Intent;
import org.chromium.base.IntentUtils;
import android.content.ComponentName;
import android.app.PendingIntent;
import org.chromium.chrome.browser.IntentHandler;
import org.chromium.chrome.browser.browserservices.intents.WebappConstants;
import org.chromium.chrome.browser.document.ChromeLauncherActivity;
import android.net.Uri;
import org.chromium.base.ContextUtils;
import android.content.ComponentName;
import org.chromium.chrome.browser.ui.searchactivityutils.SearchActivityConstants;
import org.chromium.chrome.browser.searchwidget.SearchActivity;
import org.chromium.chrome.browser.suggestions.tile.Tile;
import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.graphics.Canvas;
import java.util.Arrays;
import org.chromium.components.webapps.ShortcutSource;
import androidx.annotation.Nullable;
import org.chromium.chrome.browser.settings.BraveSearchEngineUtils;
import org.chromium.chrome.browser.init.ChromeBrowserInitializer;
import org.chromium.components.search_engines.TemplateUrl;
import com.google.gson.Gson;
import org.chromium.chrome.browser.preferences.SharedPreferencesManager;
import com.google.gson.reflect.TypeToken;
import java.lang.reflect.Type;
import java.util.Objects;
import org.chromium.components.browser_ui.widget.RoundedIconGenerator;
import org.chromium.components.favicon.IconType;
import org.chromium.components.favicon.LargeIconBridge;
import org.chromium.components.favicon.LargeIconBridge.LargeIconCallback;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.ui.favicon.FaviconUtils;
import org.chromium.url.GURL;
import androidx.core.graphics.drawable.RoundedBitmapDrawable;
import org.chromium.ui.base.ViewUtils;
import android.widget.Toast;


public class QuickActionSearchAndBookmarkWidgetProvider extends AppWidgetProvider {

    public static String FROM_SETTINGS = "FROM_SETTINGS";

    private static final int TOTAL_TILES = 16;
    private static final int TILES_PER_ROW = 4;
    private static final int MIN_VISIBLE_HEIGHT_ROW_1 = 116;
    private static final int MIN_VISIBLE_HEIGHT_ROW_2 = 184;
    private static final int MIN_VISIBLE_HEIGHT_ROW_3 = 252;
    private static final int MIN_VISIBLE_HEIGHT_ROW_4 = 320;
    private static final int DESIRED_ICON_SIZE = 44;
    private static final int DESIRED_ICON_RADIUS = 16;

    private final static int[][][] tileViewsIdArray = new int[][][]{
            {
                {R.id.ivRow1Bookmark1Icon, R.id.tvRow1Bookmark1Name, R.id.layoutRow1Bookmark1},
                {R.id.ivRow1Bookmark2Icon, R.id.tvRow1Bookmark2Name, R.id.layoutRow1Bookmark2},
                {R.id.ivRow1Bookmark3Icon, R.id.tvRow1Bookmark3Name, R.id.layoutRow1Bookmark3},
                {R.id.ivRow1Bookmark4Icon, R.id.tvRow1Bookmark4Name, R.id.layoutRow1Bookmark4},
            },
            {
                {R.id.ivRow2Bookmark1Icon, R.id.tvRow2Bookmark1Name, R.id.layoutRow2Bookmark1},
                {R.id.ivRow2Bookmark2Icon, R.id.tvRow2Bookmark2Name, R.id.layoutRow2Bookmark2},
                {R.id.ivRow2Bookmark3Icon, R.id.tvRow2Bookmark3Name, R.id.layoutRow2Bookmark3},
                {R.id.ivRow2Bookmark4Icon, R.id.tvRow2Bookmark4Name, R.id.layoutRow2Bookmark4},
            },
            {
                {R.id.ivRow3Bookmark1Icon, R.id.tvRow3Bookmark1Name, R.id.layoutRow3Bookmark1},
                {R.id.ivRow3Bookmark2Icon, R.id.tvRow3Bookmark2Name, R.id.layoutRow3Bookmark2},
                {R.id.ivRow3Bookmark3Icon, R.id.tvRow3Bookmark3Name, R.id.layoutRow3Bookmark3},
                {R.id.ivRow3Bookmark4Icon, R.id.tvRow3Bookmark4Name, R.id.layoutRow3Bookmark4},
            },
            {
                {R.id.ivRow4Bookmark1Icon, R.id.tvRow4Bookmark1Name, R.id.layoutRow4Bookmark1},
                {R.id.ivRow4Bookmark2Icon, R.id.tvRow4Bookmark2Name, R.id.layoutRow4Bookmark2},
                {R.id.ivRow4Bookmark3Icon, R.id.tvRow4Bookmark3Name, R.id.layoutRow4Bookmark3},
                {R.id.ivRow4Bookmark4Icon, R.id.tvRow4Bookmark4Name, R.id.layoutRow4Bookmark4},
            },
        };

    public QuickActionSearchAndBookmarkWidgetProvider() {
        ChromeBrowserInitializer.getInstance().handleSynchronousStartup();
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        super.onReceive(context, intent);
        String widgetAddedToHomeScreen = context.getString(R.string.widget_added_to_home_screen);
        boolean isComingFromSettings = intent.getBooleanExtra(FROM_SETTINGS, false);
        if (isComingFromSettings) {
            Toast.makeText(context, widgetAddedToHomeScreen, Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void onUpdate(Context context, AppWidgetManager appWidgetManager, int[] appWidgetIds) {
        super.onUpdate(context, appWidgetManager, appWidgetIds);
        updateAppWidgets(appWidgetIds);
    }

    @Override
    public void onAppWidgetOptionsChanged(Context context,
                                          AppWidgetManager appWidgetManager,
                                          int appWidgetId,
                                          Bundle newOptions) {
        super.onAppWidgetOptionsChanged(context, appWidgetManager, appWidgetId, newOptions);
        updateAppWidgets(new int[] { appWidgetId });
    }

    public static void updateTileIcon(Tile tile) {
        int index = indexOf(tile);
        if (index != -1)
            updateTileIcon(tileViewsIdArray[index / TILES_PER_ROW][index % TILES_PER_ROW][0], getBitmap(tile.getIcon()));
    }

    public static void updateSearchEngine(String searchEngine) {
        Context context = ContextUtils.getApplicationContext();
        AppWidgetManager appWidgetManager = AppWidgetManager.getInstance(context);
        int[] appWidgetIds = getAppWidgetIds(context, appWidgetManager);
        for (int appWidgetId : appWidgetIds) {
            RemoteViews views = getBaseRemoteViews();
            String searchWithDefaultSearchEngine = context.getString(R.string.search_with_search_engine, searchEngine);
            views.setTextViewText(R.id.tvSearchWithBrave, searchWithDefaultSearchEngine);
            appWidgetManager.partiallyUpdateAppWidget(appWidgetId, views);
        }
    }

    private static void updateAppWidgets() {
        Context context = ContextUtils.getApplicationContext();
        AppWidgetManager appWidgetManager = AppWidgetManager.getInstance(context);
        updateAppWidgets(getAppWidgetIds(context, appWidgetManager));
    }

    private static int indexOf(Tile tile) {
        List<WidgetTile> widgetTileList = DataManager.readWidgetTiles();
        for (int i = 0; i < widgetTileList.size(); i++) {
            if (widgetTileList.get(i).equals(tile))
                return i;
        }
        return -1;
    }

    private static void updateTileIcon(int imageView, Bitmap icon) {
        Context context = ContextUtils.getApplicationContext();
        AppWidgetManager appWidgetManager = AppWidgetManager.getInstance(context);
        int[] appWidgetIds = getAppWidgetIds(context, appWidgetManager);
        for (int appWidgetId : appWidgetIds) {
            RemoteViews views = getBaseRemoteViews();
            views.setImageViewBitmap(imageView, icon);
            appWidgetManager.partiallyUpdateAppWidget(appWidgetId, views);
        }
    }

    private static RemoteViews getBaseRemoteViews() {
        return new RemoteViews(ContextUtils.getApplicationContext().getPackageName(), R.layout.quick_action_search_and_bookmark_widget_layout);
    }

    private static int[] getAppWidgetIds(Context context, AppWidgetManager appWidgetManager) {
        return appWidgetManager.getAppWidgetIds(new ComponentName(context, QuickActionSearchAndBookmarkWidgetProvider.class));
    }

    private static void updateAppWidgets(int[] appWidgetIds) {
        Context context = ContextUtils.getApplicationContext();
        AppWidgetManager appWidgetManager = AppWidgetManager.getInstance(context);
        List<WidgetTile> widgetTileList = DataManager.readWidgetTiles();
        for (int appWidgetId : appWidgetIds) {
            RemoteViews views = getBaseRemoteViews();
            Bundle options = appWidgetManager.getAppWidgetOptions(appWidgetId);
            int minHeight = options.getInt(AppWidgetManager.OPTION_APPWIDGET_MIN_HEIGHT);
            setDefaultSearchEngineString(views);
            setSearchBarPendingIntent(context, views);
            setTopTiles(context, views, widgetTileList);
            setRowsVisibility(views, widgetTileList.size(), minHeight);
            appWidgetManager.updateAppWidget(appWidgetId, views);
        }
    }

    private static void setDefaultSearchEngineString(RemoteViews views) {
        TemplateUrl templateUrl = BraveSearchEngineUtils.getTemplateUrlByShortName(BraveSearchEngineUtils.getDSEShortName(false));
        if (templateUrl != null) {
            String searchWithDefaultSearchEngine = ContextUtils.getApplicationContext().getString(R.string.search_with_search_engine, templateUrl.getShortName());
            views.setTextViewText(R.id.tvSearchWithBrave, searchWithDefaultSearchEngine);
        }
    }

    private static void setTopTiles(Context context, RemoteViews views, List<WidgetTile> widgetTileList) {
        int tilesSize = widgetTileList.size();
        int i = 0;
        int j = 0;
       
        while (i < tilesSize && i < TOTAL_TILES) {
            j = j % TILES_PER_ROW;
            int row = i / TILES_PER_ROW;
            
            WidgetTile tile = widgetTileList.get(i);
            int tileLayoutId = tileViewsIdArray[row][j][2];
            int tileImageViewId = tileViewsIdArray[row][j][0];
            int tileTextViewId = tileViewsIdArray[row][j][1];

            views.setViewVisibility(tileLayoutId, View.VISIBLE);
            views.setOnClickPendingIntent(tileLayoutId, createIntent(context, tile.getUrl()));
            views.setTextViewText(tileTextViewId, tile.getTitle());
            views.setInt(tileImageViewId, "setColorFilter", 0);
            fetchGurlIcon(tileImageViewId, tile.getGURL());
            
            i++;
            j++;
        }

        //hide and uninitialize the remaining placeholder tiles
        while (i < TOTAL_TILES) {
            j = j % TILES_PER_ROW;
            int row = i / TILES_PER_ROW;
            int tileLayoutId = tileViewsIdArray[row][j][2];
            int tileImageViewId = tileViewsIdArray[row][j][0];
            int tileTextViewId = tileViewsIdArray[row][j][1];
            views.setViewVisibility(tileLayoutId, View.INVISIBLE);
            views.setOnClickPendingIntent(tileLayoutId, null);
            views.setTextViewText(tileTextViewId, "");
            views.setImageViewResource(tileImageViewId, 0);
            i++;
            j++;
        }
    }

    private static void fetchGurlIcon(final int imageViewId, GURL gurl) {
        LargeIconBridge largeIconBridge = new LargeIconBridge(Profile.getLastUsedRegularProfile());
        LargeIconCallback callback = new LargeIconCallback() {
                @Override
                public void onLargeIconAvailable(Bitmap icon, int fallbackColor,
                        boolean isFallbackColorDefault, @IconType int iconType) {
                    if (icon == null)
                        updateTileIcon(imageViewId, getTileIconFromColor(gurl, fallbackColor, isFallbackColorDefault));
                    else
                        updateTileIcon(imageViewId, getRoundedTileIconFromBitmap(icon));
                }
            };
        largeIconBridge.getLargeIconForUrl(gurl, DESIRED_ICON_SIZE, callback);
    }

    private static Bitmap getRoundedTileIconFromBitmap(Bitmap icon) {
        RoundedBitmapDrawable roundedIcon =
                ViewUtils.createRoundedBitmapDrawable(ContextUtils.getApplicationContext().getResources(), icon, DESIRED_ICON_RADIUS);
        roundedIcon.setAntiAlias(true);
        roundedIcon.setFilterBitmap(true);
        return getBitmap(roundedIcon);
    }

    private static Bitmap getTileIconFromColor(GURL gurl, int fallbackColor, boolean isFallbackColorDefault) {
        RoundedIconGenerator mIconGenerator = FaviconUtils.createRoundedRectangleIconGenerator(ContextUtils.getApplicationContext());
        mIconGenerator.setBackgroundColor(fallbackColor);
        return mIconGenerator.generateIconForUrl(gurl);
    }

    private static void setSearchBarPendingIntent(Context context, RemoteViews views) {
        views.setOnClickPendingIntent(R.id.ivIncognito, createIncognitoIntent(context));
        views.setOnClickPendingIntent(R.id.layoutSearchWithBrave, createIntent(context, false));
        views.setOnClickPendingIntent(R.id.ivVoiceSearch, createIntent(context, true));
    }

    private static Bitmap getBitmap(@Nullable Drawable drawable) {
        if (drawable != null) {
            Bitmap bitmap = Bitmap.createBitmap(drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
            Canvas canvas = new Canvas(bitmap);
            drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
            drawable.draw(canvas);
            return bitmap;
        } else 
            return null;
    }

    private static void setRowsVisibility(RemoteViews views, int tilesSize, int minHeight) {
        views.setViewVisibility(R.id.BookmarkLayoutRow1, tilesSize > 0 * TILES_PER_ROW && minHeight >= MIN_VISIBLE_HEIGHT_ROW_1 ? View.VISIBLE : View.GONE);
        views.setViewVisibility(R.id.BookmarkLayoutRow2, tilesSize > 1 * TILES_PER_ROW && minHeight >= MIN_VISIBLE_HEIGHT_ROW_2 ? View.VISIBLE : View.GONE);
        views.setViewVisibility(R.id.BookmarkLayoutRow3, tilesSize > 2 * TILES_PER_ROW && minHeight >= MIN_VISIBLE_HEIGHT_ROW_3 ? View.VISIBLE : View.GONE);
        views.setViewVisibility(R.id.BookmarkLayoutRow4, tilesSize > 3 * TILES_PER_ROW && minHeight >= MIN_VISIBLE_HEIGHT_ROW_4 ? View.VISIBLE : View.GONE);
    }

    private static PendingIntent createIntent(Context context, String url) {
        Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url), context, ChromeLauncherActivity.class);
        intent.addCategory(Intent.CATEGORY_BROWSABLE);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(WebappConstants.EXTRA_SOURCE, ShortcutSource.BOOKMARK_NAVIGATOR_WIDGET);
        intent.putExtra(WebappConstants.REUSE_URL_MATCHING_TAB_ELSE_NEW_TAB, true);
        return createPendingIntent(context, intent);
    }

    private static PendingIntent createIntent(Context context, boolean startVoiceSearch) {
        Intent searchIntent = new Intent(startVoiceSearch ? SearchActivityConstants.ACTION_START_VOICE_SEARCH
                                            : SearchActivityConstants.ACTION_START_TEXT_SEARCH);
        searchIntent.setComponent(new ComponentName(context, SearchActivity.class));
        searchIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        searchIntent.putExtra(
                SearchActivityConstants.EXTRA_BOOLEAN_FROM_QUICK_ACTION_SEARCH_WIDGET, true);
        return createPendingIntent(context, searchIntent);
    }

    private static PendingIntent createIncognitoIntent(Context context) {
        Intent trustedIncognitoIntent =
                IntentHandler.createTrustedOpenNewTabIntent(context, /*incognito=*/true);
        trustedIncognitoIntent.putExtra(IntentHandler.EXTRA_INVOKED_FROM_APP_WIDGET, true);
        trustedIncognitoIntent.addFlags(
                Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_NEW_DOCUMENT);
        trustedIncognitoIntent.putExtra(
                SearchActivityConstants.EXTRA_BOOLEAN_FROM_QUICK_ACTION_SEARCH_WIDGET, true);
        return createPendingIntent(context, trustedIncognitoIntent);
    }

    private static PendingIntent createPendingIntent(Context context, Intent intent) {
        return PendingIntent.getActivity(context, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT
                        | IntentUtils.getPendingIntentMutabilityFlag(false));
    }

    /**
     * This class acts as single source of truth for this widet. This widget would use this class to fetch the data.
     * Also, any modification of the data would be done through this class.
     **/

    public static class DataManager {
        private static final String TILES = "org.chromium.chrome.browser.widget.quickactionsearchandbookmark.QuickActionSearchAndBookmarkWidgetProvider.TILES";
        public static void parseTilesAndWriteWidgetTiles(List<Tile> tiles) {
            List<WidgetTile> widgetTileList = new ArrayList<>();
            for (Tile tile : tiles) {
                WidgetTile widgetTile = new WidgetTile(tile.getTitle(), tile.getUrl());
                widgetTileList.add(widgetTile);
            }
            writeWidgetTiles(widgetTileList);
        }

        public static void writeWidgetTiles(List<WidgetTile> widgetTileList) {
            SharedPreferencesManager.getInstance().writeString(TILES, new Gson().toJson(widgetTileList));
            updateAppWidgets();
        }

        public static List<WidgetTile> readWidgetTiles() {
            String widgetTilesJson = SharedPreferencesManager.getInstance().readString(TILES, null);
            if (widgetTilesJson != null) {
                Type type = TypeToken.getParameterized(List.class, WidgetTile.class).getType();
                return new Gson().fromJson(widgetTilesJson, type);
            }
            return new ArrayList<>();
        }
    }

    /**
     * A short class for tile. It keeps only information needed to this widget.
     **/

    public static class WidgetTile {
        private String title;
        private GURL gurl;

        public WidgetTile(String title, GURL gurl) {
            this.title = title;
            this.gurl = gurl;
        }

        public String getUrl() {
            return getGURL() != null ? getGURL().getSpec() : null;
        }

        public GURL getGURL() {
            return this.gurl;
        }

        public String getTitle() {
            return this.title;
        }

        public void parseTile(Tile tile) {
            this.gurl = tile.getUrl();
            this.title = tile.getTitle();
        }

        @Override
        public boolean equals(@Nullable Object obj) {
            if (obj instanceof WidgetTile) {
                return Objects.equals(getUrl(), ((WidgetTile) obj).getUrl());
            }
            else if (obj instanceof Tile) {
                Tile tile = (Tile) obj;
                if (tile.getUrl() != null) 
                    return Objects.equals(getUrl(), tile.getUrl().getSpec());
            }
            else if (obj instanceof String) {
                return Objects.equals(getUrl(), (String) obj);
            }
            return false;
        }

        @Override
        public int hashCode() {
            return Objects.hashCode(this.getUrl());
        }
    }
}